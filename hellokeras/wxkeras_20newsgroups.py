#!/usr/bin/env python3
import wx
import wx.grid
import keras
import os
import threading
from sklearn.metrics import roc_auc_score
import numpy
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg
from matplotlib.figure import Figure
from sklearn.datasets import fetch_20newsgroups
from keras.utils import plot_model
from keras.preprocessing.sequence import pad_sequences
from keras.models import Sequential, model_from_yaml
from keras.layers import Dense, Dropout, Activation
from keras.callbacks import ModelCheckpoint, EarlyStopping, Callback
from keras.preprocessing.text import Tokenizer
from keras import backend

class TrainingDoneEvent(wx.PyCommandEvent):
    EVT_WORK_DONE_TYPE = wx.NewEventType()
    def __init__(self, etype, eid, value=None):
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        return self._value

class EarlyStoppingByVal(Callback):
    def __init__(self, verbose=1):
        super(Callback, self).__init__()
        self.val_loss = 999.0
        self.val_acc = 0.0
        self.verbose = verbose

    def on_epoch_end(self, epoch, logs={}):
        val_loss    = logs.get('val_loss')
        val_acc     = logs.get('val_acc')
        if val_loss > self.val_loss and val_acc < self.val_acc:
            if self.verbose > 0:
                print("Epoch %05d: early stopping" % epoch)
            self.model.stop_training = True
        else:
            self.val_acc = val_acc
            self.val_loss = val_loss

class MyPanel(wx.Panel):
    def prepare_default_values(self):
        configs=[['output_dir', 'model_output/20newsgroups/dense']]
        configs.append(['epochs', '20'])
        configs.append(['batch_size', '128'])
        configs.append(['n_dense', '512'])
        configs.append(['dropout_1_rate', '0.5'])
        configs.append(['max_words', '15000'])
        return configs
    
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        configs = self.prepare_default_values()
        self.grid.CreateGrid(len(configs), 3)
        self.grid.SetColLabelValue(0, 'Name')    
        self.grid.SetColLabelValue(1, 'Default')
        self.grid.SetColLabelValue(2, 'Value')
        self.grid.SetColSize(0, 150)
        self.grid.SetColSize(1, 200)
        self.grid.SetColSize(2, 200)
        for i in range(0, len(configs)):
            self.set_row(i, configs[i][0], configs[i][1])
   
        selectBtn = wx.Button(self, label="Start")
        selectBtn.Bind(wx.EVT_BUTTON, self.onStart)
        self.history=None
        self.figure = Figure()
        self.axes = self.figure.add_subplot(121)
        self.axs = self.figure.add_subplot(1, 2, 2)
        self.canvas = FigureCanvasWxAgg(self, -1, self.figure)
        
        bagSizer    = wx.GridBagSizer(hgap=5, vgap=5)
        bagSizer.Add(self.grid, pos=(0,0),
                     flag=wx.EXPAND|wx.ALL,
                     border=1)
        bagSizer.Add(selectBtn, pos=(0,1),
                     flag=wx.ALL|wx.ALIGN_CENTER_VERTICAL,
                     border=1)      
        bagSizer.AddGrowableCol(0, 0)
        bagSizer.Add(self.canvas, flag=wx.ALL|wx.EXPAND, pos=(1,0), span=(1,2))
        bagSizer.AddGrowableRow(0, 0)
        bagSizer.AddGrowableRow(1, 0)
        self.SetSizerAndFit(bagSizer)
        
        EVT_WORK_DONE = wx.PyEventBinder(TrainingDoneEvent.EVT_WORK_DONE_TYPE, 1)
        self.Bind(EVT_WORK_DONE, self.OnDone)
        
    def onStart(self, event):
        hyperparams={}
        for row in range(0, self.grid.GetNumberRows()):
            user_value = self.grid.GetCellValue(row, 2)
            if len(user_value) < 1:
                user_value = self.grid.GetCellValue(row, 1)
            hyperparams[self.grid.GetCellValue(row, 0)] = user_value
        self.hyperparams = hyperparams
        worker = TrainingThread(self)
        worker.start()

    def set_row(self, row, txt, default_value):
        col=0
        self.grid.SetCellValue(row, col, txt)
        col=1
        self.grid.SetCellValue(row, col, str(default_value))
        self.grid.SetReadOnly(row, col)
    def OnDone(self, evt):
        self.plot_model_history(self.history)
        self.canvas.draw()
#         wx.CallAfter(self.onLongRunDone)
    def plot_model_history(self, model_history):
        axs = self.axs
        axs.plot(range(1,len(model_history.history['acc'])+1),model_history.history['acc'])
        axs.plot(range(1,len(model_history.history['val_acc'])+1),model_history.history['val_acc'])
        axs.set_title('Model Accuracy')
        axs.set_ylabel('Accuracy')
        axs.set_xlabel('Epoch')
        axs.set_xticks(numpy.arange(1,len(model_history.history['acc'])+1),len(model_history.history['acc'])/10)
        axs.legend(['train', 'val'], loc='best')
        # summarize history for loss
        axes = self.axes
        axes.plot(range(1,len(model_history.history['loss'])+1),model_history.history['loss'])
        axes.plot(range(1,len(model_history.history['val_loss'])+1),model_history.history['val_loss'])
        axes.set_title('Model Loss')
        axes.set_ylabel('Loss')
        axes.set_xlabel('Epoch')
        axes.set_xticks(numpy.arange(1,len(model_history.history['loss'])+1),len(model_history.history['loss'])/10)
        axes.legend(['train', 'val'], loc='best')
        
class MyFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, parent=None, title="Keras imdb sentiment analysis", size=wx.Size(960, 800))
        panel = MyPanel(self)
        panel.Fit()
        self.Show()

def best_epoch(model_history):
    highest_val_acc = 0
    best_epoch_num = 0
    for i in range(0, len(model_history.history['val_acc'])):
        if model_history.history['val_acc'][i]>highest_val_acc:
            highest_val_acc = model_history.history['val_acc'][i]
            best_epoch_num = i
    # print('best epoch is {} best val_acc {}'.format(best_epoch, highest_val_acc))
    # print('val_acc ', model_history.history['val_acc'])
    return best_epoch_num

class TrainingThread(threading.Thread):
    def __init__(self, parent):
        """
        @param parent: The gui object that should recieve the value
        """
        threading.Thread.__init__(self)
        self._parent = parent

    def run(self):
        hyperparams = self._parent.hyperparams
        output_dir = hyperparams['output_dir']
        epochs = int(hyperparams['epochs'])
        batch_size = int(hyperparams['batch_size'])
        n_dense = int(hyperparams['n_dense']) 
        dropout_1_rate = float(hyperparams['dropout_1_rate']) 
        max_words = int(hyperparams['max_words'])
        
        backend.clear_session()
        
        newsgroups_train = fetch_20newsgroups(subset='train')
        newsgroups_test = fetch_20newsgroups(subset='test')
        
        tokenizer = keras.preprocessing.text.Tokenizer(num_words=max_words)
        tokenizer.fit_on_texts(newsgroups_train["data"])
        
        x_train = tokenizer.texts_to_matrix(newsgroups_train["data"], mode='binary')
        x_test = tokenizer.texts_to_matrix(newsgroups_test["data"], mode='binary')
        
        num_classes = numpy.max(newsgroups_train["target"])+1
        print("num_classes is ", num_classes)
        
        y_train = keras.utils.to_categorical(newsgroups_train["target"], num_classes)
        y_test = keras.utils.to_categorical(newsgroups_test["target"], num_classes)
        
        model = Sequential()
        model.add(Dense(n_dense, input_shape=(max_words,)))
        model.add(Activation('relu'))
        model.add(Dropout(dropout_1_rate))
        model.add(Dense(num_classes))
        model.add(Activation('softmax'))
        
        yaml_string = model.to_yaml()
        model = model_from_yaml(yaml_string)
        
        plot_model(model, to_file='20newsgroups-model.png', show_shapes=True)
        
        model.compile(loss='categorical_crossentropy',
                      optimizer='adam',
                      metrics=['accuracy'])
        
        # early_stopping=EarlyStopping(monitor='val_loss')
        early_stopping = EarlyStoppingByVal()
        
        modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        self._parent.history = model.fit(x_train, y_train,
                            batch_size=batch_size,
                            epochs=epochs,
                            verbose=1,
                            validation_split=0.2,
                            callbacks=[early_stopping, modelcheckpoint])
        
        score = model.evaluate(x_test, y_test, verbose=0)
        print('epochs={}, batch_size={} Test loss: {:0.2f}, Test accuracy: {:0.2f}'.format(
                    epochs, batch_size, score[0], score[1]))
        
        model.summary()
        
#         model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])
#         modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
#         if not os.path.exists(output_dir):
#             os.makedirs(output_dir)
#         
#         self._parent.history=model.fit(x_train, y_train, batch_size=batch_size, epochs=epochs, 
#                                        verbose=1, validation_data=(x_valid, y_valid), 
#                                        callbacks=[modelcheckpoint])

        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);
        print("Best weights file: " + weights_filepath)
        model.load_weights(weights_filepath)
#         y_hat = model.predict_proba(x_valid)
        for cfg in model.get_config():
            print(cfg)
        evt = TrainingDoneEvent(TrainingDoneEvent.EVT_WORK_DONE_TYPE, -1)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()
