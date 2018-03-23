#!/usr/bin/env python3
import wx
import wx.grid
import keras
from keras.datasets import imdb
from keras.preprocessing.sequence import pad_sequences
from keras.models import Sequential
from keras.layers import Dense, Dropout, Embedding, SpatialDropout1D, GlobalMaxPooling1D
from keras.layers import Conv1D, MaxPooling1D 
from keras.callbacks import ModelCheckpoint
import os
import threading
from sklearn.metrics import roc_auc_score
import numpy
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg
from matplotlib.figure import Figure

class TrainingDoneEvent(wx.PyCommandEvent):
    EVT_WORK_DONE_TYPE = wx.NewEventType()
    def __init__(self, etype, eid, value=None):
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        return self._value

class MyPanel(wx.Panel):
    def prepare_default_values(self):
        configs=[['output_dir', 'model_output/imdb/cnn']]
        configs.append(['epochs', '4'])
        configs.append(['batch_size', '128'])
        configs.append(['n_dim', '64'])
        configs.append(['embedding_1_input_dim', '5000'])
        configs.append(['max_review_length', '400'])
        configs.append(['pad_type', 'pre'])
        
        configs.append(['trunc_type', 'pre'])
        configs.append(['drop_embed', '0.2'])
        configs.append(['n_conv', '256'])
        configs.append(['k_conv', '3'])
        configs.append(['n_dense', '256'])
        configs.append(['dropout_1_rate', '0.2'])
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
        self.y_hat=None #placeholder
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

    def onLongRunDone(self):
        self.axes.hist(self.y_hat)
        self.axes.axvline(x=0.5, color='orange')
        self.canvas.draw()
        
    def set_row(self, row, txt, default_value):
        col=0
        self.grid.SetCellValue(row, col, txt)
        col=1
        self.grid.SetCellValue(row, col, str(default_value))
        self.grid.SetReadOnly(row, col)
    def OnDone(self, evt):
        self.y_hat = evt.GetValue()
        self.axes.hist(self.y_hat)
        self.axes.axvline(x=0.5, color='orange')
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
        
        # vector-space embedding: 
        n_dim = int(hyperparams['n_dim']) 
        embedding_1_input_dim = int(hyperparams['embedding_1_input_dim']) 
        max_review_length = int(hyperparams['max_review_length']) 
        pad_type = hyperparams['pad_type']
        trunc_type = hyperparams['trunc_type']
        drop_embed = float(hyperparams['drop_embed']) 
        
        # convolutional layer architecture:
        n_conv = int(hyperparams['n_conv'])  
        k_conv = int(hyperparams['k_conv']) 
        
        n_dense = int(hyperparams['n_dense']) 
        dropout_1_rate = float(hyperparams['dropout_1_rate'])
        
        (x_train, y_train), (x_valid, y_valid) = imdb.load_data(num_words=embedding_1_input_dim)
        x_train = pad_sequences(x_train, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        x_valid = pad_sequences(x_valid, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        
        model = Sequential()
        model.add(Embedding(embedding_1_input_dim, n_dim, input_length=max_review_length)) 
        model.add(SpatialDropout1D(drop_embed))
        model.add(Conv1D(n_conv, k_conv, activation='relu'))
        model.add(GlobalMaxPooling1D())
        model.add(Dense(n_dense, activation='relu'))
        model.add(Dropout(dropout_1_rate))
        model.add(Dense(1, activation='sigmoid'))
        
        model.summary()
        
        model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])
        modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        self._parent.history=model.fit(x_train, y_train, batch_size=batch_size, epochs=epochs, 
                                       verbose=1, validation_data=(x_valid, y_valid), 
                                       callbacks=[modelcheckpoint])

        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);
        print("Using " + weights_filepath)
        model.load_weights(weights_filepath)
        y_hat = model.predict_proba(x_valid)
        for cfg in model.get_config():
            print(cfg)
        print("epochs={}, batch_size={} roc_auc_score {:0.2f}".format(epochs, batch_size, roc_auc_score(y_valid, y_hat)*100.0))
        evt = TrainingDoneEvent(TrainingDoneEvent.EVT_WORK_DONE_TYPE, -1, y_hat)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()

"""
sudo pip install -U -f https://extras.wxpython.org/wxPython4/extras/linux/gtk3/ubuntu-16.04 wxPython
sudo apt-get install libsdl-ttf2.0-0
sudo pip3 install -U keras
sudo pip3 install -U tensorflow
sudo pip3 install -U sklearn matplotlib h5py



"""
