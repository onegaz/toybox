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
    def prepare_defaults(self):
        configs=[]
        configs.append(['output_dir', 'model_output/biLSTM'])
        configs.append(['epochs', '6'])
        configs.append(['batch_size', '128'])
        configs.append(['n_dim', '64'])
        configs.append(['n_unique_words', '10000'])
        configs.append(['max_review_length', '200'])
        configs.append(['pad_type', 'pre'])
        configs.append(['trunc_type', 'pre'])
        configs.append(['drop_embed', '0.2'])
        configs.append(['n_lstm', '256'])
        configs.append(['drop_lstm', '0.2'])
        return configs        
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        configs = self.prepare_defaults()
        self.grid.CreateGrid(len(configs), 3)
#         self.grid.CreateGrid(11, 3)
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
        # LSTM layer architecture:
        n_lstm = int(hyperparams['n_lstm']) 
        drop_lstm = float(hyperparams['drop_lstm'])
        
        (x_train, y_train), (x_valid, y_valid) = imdb.load_data(num_words=n_unique_words)
        x_train = pad_sequences(x_train, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        x_valid = pad_sequences(x_valid, maxlen=max_review_length, padding=pad_type, truncating=trunc_type, value=0)
        
        model = Sequential()
        model.add(Embedding(n_unique_words, n_dim, input_length=max_review_length)) 
        model.add(SpatialDropout1D(drop_embed))
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
        self._parent.history=model.fit(x_train, y_train, batch_size=batch_size, 
                                       epochs=epochs, verbose=1, 
                                       validation_data=(x_valid, y_valid), 
                                       callbacks=[modelcheckpoint])

        weights_filepath = output_dir+"/weights.{:02d}.hdf5".format(best_epoch(self._parent.history)+1);
        print("Using " + weights_filepath)
        model.load_weights(weights_filepath)
        y_hat = model.predict_proba(x_valid)
        for cfg in model.get_config():
            print(cfg)
        print("epochs={}, batch_size={} roc_auc_score {:0.2f}".format(epochs, 
                                                                      batch_size, 
                                                                      roc_auc_score(y_valid, y_hat)*100.0))
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

~/oss/toybox/hellokeras$ ./wxkeras_imdb_bi_lstm.py
/usr/local/lib/python3.5/dist-packages/h5py/__init__.py:36: FutureWarning: Conversion of the second argument of issubdtype from `float` to `np.floating` is deprecated. In future, it will be treated as `np.float64 == np.dtype(float).type`.
  from ._conv import register_converters as _register_converters
Using TensorFlow backend.
_________________________________________________________________
Layer (type)                 Output Shape              Param #   
=================================================================
embedding_1 (Embedding)      (None, 200, 64)           640000    
_________________________________________________________________
spatial_dropout1d_1 (Spatial (None, 200, 64)           0         
_________________________________________________________________
bidirectional_1 (Bidirection (None, 512)               657408    
_________________________________________________________________
dense_1 (Dense)              (None, 1)                 513       
=================================================================
Total params: 1,297,921
Trainable params: 1,297,921
Non-trainable params: 0
_________________________________________________________________
Train on 25000 samples, validate on 25000 samples
Epoch 1/6
2018-03-20 21:55:43.753454: I tensorflow/core/platform/cpu_feature_guard.cc:140] Your CPU supports instructions that this TensorFlow binary was not compiled to use: AVX2
25000/25000 [==============================] - 355s 14ms/step - loss: 0.6348 - acc: 0.6564 - val_loss: 0.7930 - val_acc: 0.6649
Epoch 2/6
25000/25000 [==============================] - 364s 15ms/step - loss: 0.3690 - acc: 0.8451 - val_loss: 0.3439 - val_acc: 0.8554
Epoch 3/6
25000/25000 [==============================] - 361s 14ms/step - loss: 0.2515 - acc: 0.9026 - val_loss: 0.3323 - val_acc: 0.8682
Epoch 4/6
25000/25000 [==============================] - 355s 14ms/step - loss: 0.2046 - acc: 0.9236 - val_loss: 0.3205 - val_acc: 0.8717
Epoch 5/6
25000/25000 [==============================] - 352s 14ms/step - loss: 0.1686 - acc: 0.9380 - val_loss: 0.3966 - val_acc: 0.8520
Epoch 6/6
25000/25000 [==============================] - 372s 15ms/step - loss: 0.1404 - acc: 0.9502 - val_loss: 0.3558 - val_acc: 0.8635
Using model_output/biLSTM/weights.04.hdf5
{'class_name': 'Embedding', 'config': {'embeddings_initializer': {'class_name': 'RandomUniform', 'config': {'seed': None, 'minval': -0.05, 'maxval': 0.05}}, 'mask_zero': False, 'embeddings_regularizer': None, 'activity_regularizer': None, 'output_dim': 64, 'dtype': 'float32', 'embeddings_constraint': None, 'batch_input_shape': (None, 200), 'input_dim': 10000, 'trainable': True, 'name': 'embedding_1', 'input_length': 200}}
{'class_name': 'SpatialDropout1D', 'config': {'rate': 0.2, 'seed': None, 'trainable': True, 'name': 'spatial_dropout1d_1', 'noise_shape': None}}
{'class_name': 'Bidirectional', 'config': {'merge_mode': 'concat', 'layer': {'class_name': 'LSTM', 'config': {'recurrent_dropout': 0.0, 'unroll': False, 'bias_regularizer': None, 'bias_initializer': {'class_name': 'Zeros', 'config': {}}, 'activity_regularizer': None, 'recurrent_initializer': {'class_name': 'Orthogonal', 'config': {'seed': None, 'gain': 1.0}}, 'stateful': False, 'units': 256, 'kernel_regularizer': None, 'bias_constraint': None, 'kernel_constraint': None, 'trainable': True, 'implementation': 1, 'go_backwards': False, 'name': 'lstm_1', 'recurrent_activation': 'hard_sigmoid', 'kernel_initializer': {'class_name': 'VarianceScaling', 'config': {'seed': None, 'scale': 1.0, 'mode': 'fan_avg', 'distribution': 'uniform'}}, 'activation': 'tanh', 'use_bias': True, 'recurrent_constraint': None, 'recurrent_regularizer': None, 'return_sequences': False, 'dropout': 0.2, 'return_state': False, 'unit_forget_bias': True}}, 'trainable': True, 'name': 'bidirectional_1'}}
{'class_name': 'Dense', 'config': {'bias_regularizer': None, 'kernel_constraint': None, 'bias_initializer': {'class_name': 'Zeros', 'config': {}}, 'bias_constraint': None, 'kernel_initializer': {'class_name': 'VarianceScaling', 'config': {'seed': None, 'scale': 1.0, 'mode': 'fan_avg', 'distribution': 'uniform'}}, 'activation': 'sigmoid', 'units': 1, 'kernel_regularizer': None, 'trainable': True, 'name': 'dense_1', 'activity_regularizer': None, 'use_bias': True}}
epochs=6, batch_size=128 roc_auc_score 94.15

~/oss/toybox/hellokeras$ ls model_output/biLSTM/
weights.01.hdf5  weights.02.hdf5  weights.03.hdf5  weights.04.hdf5  weights.05.hdf5  weights.06.hdf5
"""
