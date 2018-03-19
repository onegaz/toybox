#!/usr/bin/env python3
import wx
import wx.grid
import keras
from keras.datasets import imdb
from keras.preprocessing.sequence import pad_sequences
from keras.models import Sequential
from keras.layers import Dense, Dropout, Embedding, SpatialDropout1D, LSTM
from keras.layers.wrappers import Bidirectional 
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

EVT_WORK_DONE_TYPE = wx.NewEventType()

class TrainingDoneEvent(wx.PyCommandEvent):
    def __init__(self, etype, eid, value=None):
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

class MyPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        self.grid.CreateGrid(14, 3)

        row=0
        self.set_row(row, 'output_dir', 'model_output/cnn-biLSTM')
        row=row+1
        self.set_row(row, 'epochs', 3)
        row=row+1
        self.set_row(row, 'batch_size', 128)
        row=row+1
        self.set_row(row, 'n_dim', 64)
        row=row+1
        self.set_row(row, 'n_unique_words', 10000)
        row=row+1
        self.set_row(row, 'max_review_length', 100)        
        row=row+1
        self.set_row(row, 'pad_type', 'pre')        
        row=row+1
        self.set_row(row, 'trunc_type', 'pre')
        row=row+1
        self.set_row(row, 'drop_embed', 0.2)                
        row=row+1
        self.set_row(row, 'n_conv', 64)                
        row=row+1
        self.set_row(row, 'k_conv', 3)                
        row=row+1
        self.set_row(row, 'mp_size', 4)                
        row=row+1
        self.set_row(row, 'n_lstm', 64)                
        row=row+1
        self.set_row(row, 'drop_lstm', 0.2)    
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
        bagSizer.AddGrowableCol(1, 0)
        bagSizer.Add(self.canvas, flag=wx.ALL|wx.EXPAND, pos=(1,0), span=(1,2))
        bagSizer.AddGrowableRow(0, 0)
        bagSizer.AddGrowableRow(1, 0)
        self.SetSizerAndFit(bagSizer)
        
        EVT_WORK_DONE = wx.PyEventBinder(EVT_WORK_DONE_TYPE, 1)
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
        n_unique_words = int(hyperparams['n_unique_words']) 
        max_review_length = int(hyperparams['max_review_length']) 
        pad_type = hyperparams['pad_type']
        trunc_type = hyperparams['trunc_type']
        drop_embed = float(hyperparams['drop_embed']) 
        
        # convolutional layer architecture:
        n_conv = int(hyperparams['n_conv'])  
        k_conv = int(hyperparams['k_conv']) 
        mp_size = int(hyperparams['mp_size'])
        
        # LSTM layer architecture:
        n_lstm = int(hyperparams['n_lstm']) 
        drop_lstm = float(hyperparams['drop_lstm'])
        
        (x_train, y_train), (x_valid, y_valid) = imdb.load_data(num_words=n_unique_words)
        x_train = pad_sequences(x_train, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        x_valid = pad_sequences(x_valid, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        
        model = Sequential()
        model.add(Embedding(n_unique_words, n_dim, input_length=max_review_length)) 
        model.add(SpatialDropout1D(drop_embed))
        model.add(Conv1D(n_conv, k_conv, activation='relu'))
        model.add(MaxPooling1D(mp_size))
        model.add(Bidirectional(LSTM(n_lstm, dropout=drop_lstm)))
        model.add(Dense(1, activation='sigmoid'))
        
        # LSTM layer parameters double due to both reading directions
        model.summary()
        
        model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])
        modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        filepath=output_dir+"/weights.02.hdf5"
        # slightly higher than bidirectional LSTM and about the same as stacked biLSTM
        # but epochs are a third as long, or one-sixth as long, respectively
#         if not os.path.isfile(filepath):
        self._parent.history=model.fit(x_train, y_train, batch_size=batch_size, epochs=epochs, verbose=1, validation_data=(x_valid, y_valid), callbacks=[modelcheckpoint])
#         else:
#             print(filepath + ' already exists, skip training')
            
        model.load_weights(output_dir+"/weights.02.hdf5")
        y_hat = model.predict_proba(x_valid)
        for cfg in model.get_config():
            print(cfg)
        print("epochs={}, batch_size={} roc_auc_score {:0.2f}".format(epochs, batch_size, roc_auc_score(y_valid, y_hat)*100.0))
        evt = TrainingDoneEvent(EVT_WORK_DONE_TYPE, -1, y_hat)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()