#!/usr/bin/env python3
from __future__ import print_function
import os
import platform
import threading
from datetime import datetime
import wx
import wx.grid
import numpy
numpy.random.seed(1337) # for reproducibility
import keras
from keras.datasets import mnist
from keras.models import Sequential, model_from_yaml
from keras.layers import Dense, Dropout
from keras import backend
import tensorflow
from keras.callbacks import TensorBoard, Callback, ModelCheckpoint
from keras.layers.normalization import BatchNormalization
from keras import regularizers
from keras.optimizers import SGD
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
        """Returns the value from the event.
        @return: the value of this event

        """
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
        if val_loss > self.val_loss*1.1 and val_acc < self.val_acc*0.95:
            if self.verbose > 0:
                print("Epoch %05d: early stopping" % epoch)
            self.model.stop_training = True
        else:
            self.val_acc = val_acc
            self.val_loss = val_loss
            
class MyPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        configs=[['output_dir', 'model_output/mnist/norm']]
        configs.append(['epochs', '100'])
        configs.append(['batch_size', '128'])
        configs.append(['dropout_1', '0.5'])
        configs.append(['dense_1', '64'])
        configs.append(['dense_2', '64'])
        configs.append(['dropout_2', '0.5'])
        
        self.grid.CreateGrid(len(configs), 3)
        self.grid.SetColLabelValue(0, 'Name')    
        self.grid.SetColLabelValue(1, 'Default')
        self.grid.SetColLabelValue(2, 'Value')
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
        self.grid.SetColSize(0, 150)
        self.grid.SetColSize(1, 200)
        self.grid.SetColSize(2, 200)
        self.grid.ForceRefresh()
        
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
        # input image dimensions
        output_dir  = hyperparams['output_dir']
        epochs      = int(hyperparams['epochs'])
        batch_size  = int(hyperparams['batch_size'])
        dropout_1       = float(hyperparams['dropout_1'])
        dense_1         = int(hyperparams['dense_1'])
        dense_2         = int(hyperparams['dense_2'])
        dropout_2       = float(hyperparams['dropout_2'])
        
        backend.clear_session()
        # the data, split between train and test sets
        (x_train, y_train), (x_test, y_test) = mnist.load_data()
        if x_train.shape[1] != 784:
            x_train = x_train.reshape(x_train.shape[0], 784).astype('float32')
            x_test = x_test.reshape(x_test.shape[0], 784).astype('float32')
        # normalize our data values to the range [0, 1].
        x_train /= 255
        x_test /= 255    
        # convert class vectors to binary class matrices
        num_classes = numpy.max(y_train)+1
        y_train = keras.utils.to_categorical(y_train, num_classes)
        y_test = keras.utils.to_categorical(y_test, num_classes)
                     
        model = Sequential()
        model.add(Dense(dense_1, activation='relu', input_shape=(784,)))
        model.add(BatchNormalization())
        model.add(Dropout(dropout_1))
        model.add(Dense(dense_2, activation='relu'))
        model.add(BatchNormalization())
        model.add(Dropout(dropout_2))
        model.add(Dense(num_classes, activation='softmax'))

        yaml_string = model.to_yaml()
        model = model_from_yaml(yaml_string)
        
        model.summary()
        
        model.compile(loss=keras.losses.categorical_crossentropy,
                      optimizer=keras.optimizers.Adadelta(),
                      metrics=['accuracy'])
        
        tb_log_dir = os.path.abspath('tensorboard_log')
        print('tensorboard --logdir={}'.format(tb_log_dir))
        tbCallback = keras.callbacks.TensorBoard(log_dir=tb_log_dir, histogram_freq=0,  
                  write_graph=True, write_images=True)
        tbCallback.set_model(model)

        modelcheckpoint = ModelCheckpoint(filepath=output_dir+"/weights.{epoch:02d}.hdf5")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        early_stopping = EarlyStoppingByVal()
        
        start = datetime.now()
        history = model.fit(x_train, y_train,
                  batch_size=batch_size,
                  epochs=epochs,
                  verbose=1,
                  validation_data=(x_test, y_test),
                  callbacks=[modelcheckpoint, tbCallback, early_stopping])
        training_time = datetime.now() - start
        score = model.evaluate(x_test, y_test, verbose=0)
        print('epochs={}, batch_size={} Test loss: {}, Test accuracy: {}'.format(
            epochs, batch_size, score[0], score[1]))
        print('python version {}, tensorflow version {}, keras version {}'.format(
            platform.python_version(), tensorflow.__version__, keras.__version__))
        print('Model took {} seconds to train'.format(training_time.seconds))
        self._parent.history=history
        
        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);
        print("Best weights filepath: " + weights_filepath)
        for cfg in model.get_config():
            print(cfg)
        evt = TrainingDoneEvent(TrainingDoneEvent.EVT_WORK_DONE_TYPE, -1)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()

"""
sudo pip install -U -f https://extras.wxpython.org/wxPython4/extras/linux/gtk3/ubuntu-16.04 wxPython
sudo apt-get install libsdl-ttf2.0-0
sudo pip3 install -U keras tensorflow sklearn matplotlib h5py

"""
