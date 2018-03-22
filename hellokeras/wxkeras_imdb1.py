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
    def prepare_default_values(self):
        configs=[['output_dir', 'model_output/cnn-biLSTM']]
        configs.append(['epochs', '4'])
        configs.append(['batch_size', '128'])
        configs.append(['n_dim', '4'])
        configs.append(['n_unique_words', '10000'])
        configs.append(['max_review_length', '100'])
        configs.append(['pad_type', 'pre'])
        configs.append(['trunc_type', 'pre'])
        configs.append(['drop_embed', '0.2'])
        configs.append(['n_conv', '64'])
        configs.append(['k_conv', '3'])
        configs.append(['mp_size', '4'])
        configs.append(['n_lstm', '64'])
        configs.append(['drop_lstm', '0.2'])
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
        
        # slightly higher than bidirectional LSTM and about the same as stacked biLSTM
        # but epochs are a third as long, or one-sixth as long, respectively
        self._parent.history=model.fit(x_train, y_train, batch_size=batch_size, epochs=epochs, verbose=1, validation_data=(x_valid, y_valid), callbacks=[modelcheckpoint])

        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);
        print("Using " + weights_filepath)
        model.load_weights(weights_filepath)
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

"""
sudo pip install -U -f https://extras.wxpython.org/wxPython4/extras/linux/gtk3/ubuntu-16.04 wxPython
sudo apt-get install libsdl-ttf2.0-0
sudo pip3 install -U keras
sudo pip3 install -U tensorflow
sudo pip3 install -U sklearn matplotlib h5py

~/oss/toybox/hellokeras$ ./wxkeras_imdb1.py
/usr/local/lib/python3.5/dist-packages/h5py/__init__.py:36: FutureWarning: Conversion of the second argument of issubdtype from `float` to `np.floating` is deprecated. In future, it will be treated as `np.float64 == np.dtype(float).type`.
  from ._conv import register_converters as _register_converters
Using TensorFlow backend.
WARNING:tensorflow:From /usr/local/lib/python3.5/dist-packages/tensorflow/python/util/deprecation.py:497: calling conv1d (from tensorflow.python.ops.nn_ops) with data_format=NHWC is deprecated and will be removed in a future version.
Instructions for updating:
`NHWC` for data_format is deprecated, use `NWC` instead
_________________________________________________________________
Layer (type)                 Output Shape              Param #   
=================================================================
embedding_1 (Embedding)      (None, 100, 4)            40000     
_________________________________________________________________
spatial_dropout1d_1 (Spatial (None, 100, 4)            0         
_________________________________________________________________
conv1d_1 (Conv1D)            (None, 98, 64)            832       
_________________________________________________________________
max_pooling1d_1 (MaxPooling1 (None, 24, 64)            0         
_________________________________________________________________
bidirectional_1 (Bidirection (None, 128)               66048     
_________________________________________________________________
dense_1 (Dense)              (None, 1)                 129       
=================================================================
Total params: 107,009
Trainable params: 107,009
Non-trainable params: 0
_________________________________________________________________
Train on 25000 samples, validate on 25000 samples
Epoch 1/4
2018-03-21 09:47:16.468106: I tensorflow/core/platform/cpu_feature_guard.cc:140] Your CPU supports instructions that this TensorFlow binary was not compiled to use: AVX2
25000/25000 [==============================] - 16s 620us/step - loss: 0.5524 - acc: 0.6788 - val_loss: 0.3747 - val_acc: 0.8312
Epoch 2/4
25000/25000 [==============================] - 14s 556us/step - loss: 0.3377 - acc: 0.8555 - val_loss: 0.3531 - val_acc: 0.8428
Epoch 3/4
25000/25000 [==============================] - 14s 557us/step - loss: 0.2828 - acc: 0.8830 - val_loss: 0.3828 - val_acc: 0.8292
Epoch 4/4
25000/25000 [==============================] - 14s 558us/step - loss: 0.2478 - acc: 0.9033 - val_loss: 0.3714 - val_acc: 0.8357
Using model_output/cnn-biLSTM/weights.02.hdf5
{'class_name': 'Embedding', 'config': {'input_length': 100, 'batch_input_shape': (None, 100), 'embeddings_initializer': {'class_name': 'RandomUniform', 'config': {'maxval': 0.05, 'minval': -0.05, 'seed': None}}, 'dtype': 'float32', 'trainable': True, 'activity_regularizer': None, 'name': 'embedding_1', 'embeddings_regularizer': None, 'embeddings_constraint': None, 'output_dim': 4, 'mask_zero': False, 'input_dim': 10000}}
{'class_name': 'SpatialDropout1D', 'config': {'rate': 0.2, 'noise_shape': None, 'name': 'spatial_dropout1d_1', 'trainable': True, 'seed': None}}
{'class_name': 'Conv1D', 'config': {'dilation_rate': (1,), 'bias_regularizer': None, 'padding': 'valid', 'use_bias': True, 'kernel_size': (3,), 'kernel_regularizer': None, 'kernel_constraint': None, 'name': 'conv1d_1', 'filters': 64, 'trainable': True, 'bias_initializer': {'class_name': 'Zeros', 'config': {}}, 'bias_constraint': None, 'strides': (1,), 'activation': 'relu', 'kernel_initializer': {'class_name': 'VarianceScaling', 'config': {'scale': 1.0, 'mode': 'fan_avg', 'distribution': 'uniform', 'seed': None}}, 'activity_regularizer': None}}
{'class_name': 'MaxPooling1D', 'config': {'padding': 'valid', 'strides': (4,), 'trainable': True, 'name': 'max_pooling1d_1', 'pool_size': (4,)}}
{'class_name': 'Bidirectional', 'config': {'name': 'bidirectional_1', 'merge_mode': 'concat', 'trainable': True, 'layer': {'class_name': 'LSTM', 'config': {'recurrent_regularizer': None, 'unroll': False, 'recurrent_initializer': {'class_name': 'Orthogonal', 'config': {'gain': 1.0, 'seed': None}}, 'use_bias': True, 'kernel_regularizer': None, 'unit_forget_bias': True, 'stateful': False, 'bias_constraint': None, 'recurrent_dropout': 0.0, 'implementation': 1, 'dropout': 0.2, 'return_state': False, 'go_backwards': False, 'activation': 'tanh', 'bias_initializer': {'class_name': 'Zeros', 'config': {}}, 'bias_regularizer': None, 'kernel_constraint': None, 'recurrent_constraint': None, 'units': 64, 'recurrent_activation': 'hard_sigmoid', 'name': 'lstm_1', 'activity_regularizer': None, 'trainable': True, 'return_sequences': False, 'kernel_initializer': {'class_name': 'VarianceScaling', 'config': {'scale': 1.0, 'mode': 'fan_avg', 'distribution': 'uniform', 'seed': None}}}}}}
{'class_name': 'Dense', 'config': {'bias_regularizer': None, 'kernel_constraint': None, 'kernel_initializer': {'class_name': 'VarianceScaling', 'config': {'scale': 1.0, 'mode': 'fan_avg', 'distribution': 'uniform', 'seed': None}}, 'use_bias': True, 'kernel_regularizer': None, 'units': 1, 'bias_constraint': None, 'name': 'dense_1', 'activity_regularizer': None, 'trainable': True, 'activation': 'sigmoid', 'bias_initializer': {'class_name': 'Zeros', 'config': {}}}}
epochs=4, batch_size=128 roc_auc_score 92.49

"""
