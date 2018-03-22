#!/usr/bin/env python3
import wx
import wx.grid
import numpy
numpy.random.seed(42)
import keras
from tflearn.datasets import oxflower17
from keras.models import Sequential
from keras.layers import Dense, Dropout, Flatten, SpatialDropout1D, Conv2D, MaxPooling2D
from keras.callbacks import ModelCheckpoint, TensorBoard
from keras.layers.normalization import BatchNormalization
import os
import threading
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
    def prepare_defaults(self):
        configs=[]
        configs.append(['output_dir', 'model_output/alexnet-oxflower17'])
        configs.append(['epochs', '6'])
        configs.append(['batch_size', '128'])
        configs.append(['validation_split', '0.2'])
        configs.append(['conv2d_1_filters', '96'])
        configs.append(['conv2d_2_filters', '256'])
        configs.append(['conv2d_3_filters', '256'])
        configs.append(['conv2d_4_filters', '384'])
        configs.append(['conv2d_5_filters', '384'])
        configs.append(['dense_1_units', '4096'])
        configs.append(['dense_2_units', '4096'])
        configs.append(['dense_3_units', '17'])
        configs.append(['drop_1_rate', '0.5'])
        configs.append(['drop_2_rate', '0.5'])
        configs.append(['fit_verbose', '1'])
        return configs        
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        configs = self.prepare_defaults()
        self.grid.CreateGrid(len(configs), 3)
        self.grid.SetColLabelValue(0, 'Name')    
        self.grid.SetColLabelValue(1, 'Default')
        self.grid.SetColLabelValue(2, 'Value')
        self.grid.SetColSize(0, 150)
        self.grid.SetColSize(1, 240)
        self.grid.SetColSize(2, 240)
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
        wx.Frame.__init__(self, parent=None, title="Keras oxflower17 alexnet", size=wx.Size(960, 800))
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
        
        validation_split = float(hyperparams['validation_split']) 
        conv2d_1_filters = int(hyperparams['conv2d_1_filters']) 
        conv2d_2_filters = int(hyperparams['conv2d_2_filters'])
        conv2d_3_filters = int(hyperparams['conv2d_3_filters']) 
        conv2d_4_filters = int(hyperparams['conv2d_4_filters']) 
        conv2d_5_filters = int(hyperparams['conv2d_5_filters']) 
        dense_1_units = int(hyperparams['dense_1_units'])
        dense_2_units = int(hyperparams['dense_2_units'])
        dense_3_units = int(hyperparams['dense_3_units'])
        drop_1_rate = float(hyperparams['drop_1_rate'])
        drop_2_rate = float(hyperparams['drop_2_rate'])
        fit_verbose = int(hyperparams['fit_verbose']) 
        X, Y = oxflower17.load_data(one_hot=True)
        
        model = Sequential()
        model.add(Conv2D(conv2d_1_filters, kernel_size=(11, 11), strides=(4, 4), activation='relu', input_shape=(224, 224, 3)))
        model.add(MaxPooling2D(pool_size=(3, 3), strides=(2, 2)))
        model.add(BatchNormalization())
        
        model.add(Conv2D(conv2d_2_filters, kernel_size=(5, 5), activation='relu'))
        model.add(MaxPooling2D(pool_size=(3, 3), strides=(2, 2)))
        model.add(BatchNormalization())
        
        model.add(Conv2D(conv2d_3_filters, kernel_size=(3, 3), activation='relu'))
        model.add(Conv2D(conv2d_4_filters, kernel_size=(3, 3), activation='relu'))
        model.add(Conv2D(conv2d_5_filters, kernel_size=(3, 3), activation='relu'))
        model.add(MaxPooling2D(pool_size=(3, 3), strides=(2, 2)))
        model.add(BatchNormalization())
        
        model.add(Flatten())
        model.add(Dense(dense_1_units, activation='tanh'))
        model.add(Dropout(drop_1_rate))
        model.add(Dense(dense_2_units, activation='tanh'))
        model.add(Dropout(drop_2_rate))
        model.add(Dense(dense_3_units, activation='softmax'))
        
        model.summary()
        
        model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
        modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        tensorbrd = TensorBoard('tb_logs/alexnet')

        self._parent.history=model.fit(X, Y, batch_size=batch_size, 
                                       epochs=epochs, verbose=fit_verbose, 
                                       validation_split=validation_split, shuffle=True, 
                                       callbacks=[modelcheckpoint, tensorbrd])

        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);

        for cfg in model.get_config():
            print(cfg)
        print("epochs={}, batch_size={} best weights filepath {}".format(epochs, batch_size, weights_filepath))
        evt = TrainingDoneEvent(EVT_WORK_DONE_TYPE, -1)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()