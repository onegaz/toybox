#!/usr/bin/env python3
import os
import platform
import threading
from datetime import datetime
import wx
import wx.grid
import numpy
numpy.random.seed(1337) # for reproducibility
import tensorflow
tensorflow.set_random_seed(42)
from tensorflow.examples.tutorials.mnist import input_data

def get_defaults():
    hps = []
    hps.append(['learning_rate', '0.1'])
    hps.append(['epochs', '20'])
    hps.append(['batch_size', '128'])
    hps.append(['n_dense_1', '64'])
    hps.append(['n_dense_2', '64'])
    hps.append(['n_dense_3', '64'])
    hps.append(['verbose_interval', '30'])
    return hps

def dense(x, W, b):
    z = tensorflow.add(tensorflow.matmul(x, W), b)
    a = tensorflow.nn.relu(z)
    return a

def network(x, weights, biases):
    # two dense hidden layers: 
    dense_1 = dense(x, weights['W1'], biases['b1'])
    dense_2 = dense(dense_1, weights['W2'], biases['b2'])
    dense_3 = dense(dense_2, weights['W3'], biases['b3'])
    
    out_layer_z = tensorflow.add(tensorflow.matmul(dense_3, weights['W_out']), biases['b_out'])
    return out_layer_z

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

class MyPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.grid = wx.grid.Grid(self, -1)
        configs=get_defaults()
        
        self.grid.CreateGrid(len(configs), 3)
        self.grid.SetColLabelValue(0, 'Name')    
        self.grid.SetColLabelValue(1, 'Default')
        self.grid.SetColLabelValue(2, 'Value')
        for i in range(0, len(configs)):
            self.set_row(i, configs[i][0], configs[i][1])

        selectBtn = wx.Button(self, label="Start")
        selectBtn.Bind(wx.EVT_BUTTON, self.onStart)
        
        bagSizer    = wx.GridBagSizer(hgap=5, vgap=5)
        bagSizer.Add(self.grid, pos=(0,0),
                     flag=wx.EXPAND|wx.ALL,
                     border=1)
        bagSizer.Add(selectBtn, pos=(0,1),
                     flag=wx.ALL|wx.ALIGN_CENTER_VERTICAL,
                     border=1)      
        bagSizer.AddGrowableCol(0, 0)
#         bagSizer.Add(self.canvas, flag=wx.ALL|wx.EXPAND, pos=(1,0), span=(1,2))
        bagSizer.AddGrowableRow(0, 0)
#         bagSizer.AddGrowableRow(1, 0)
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
        pass
                
class MyFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, parent=None, title="Keras imdb sentiment analysis", size=wx.Size(760, 400))
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
        @param parent: The gui object that should recieve notification
        """
        threading.Thread.__init__(self)
        self._parent = parent

    def run(self):
        hyperparams = self._parent.hyperparams
        learning_rate  = float(hyperparams['learning_rate'])
        epochs      = int(hyperparams['epochs'])
        batch_size  = int(hyperparams['batch_size'])
        n_dense_1       = int(hyperparams['n_dense_1'])
        n_dense_2       = int(hyperparams['n_dense_2'])
        n_dense_3       = int(hyperparams['n_dense_3'])
        verbose_interval= int(hyperparams['verbose_interval'])
        mnist = input_data.read_data_sets("MNIST_data/", one_hot=True)

        num_features = 784
        n_classes = 10
        x = tensorflow.placeholder(tensorflow.float32, [None, num_features])
        y = tensorflow.placeholder(tensorflow.float32, [None, n_classes])
        weight_initializer = tensorflow.contrib.layers.xavier_initializer()
        
        bias_dict = {
            'b1': tensorflow.Variable(tensorflow.zeros([n_dense_1])), 
            'b2': tensorflow.Variable(tensorflow.zeros([n_dense_2])),
            'b3': tensorflow.Variable(tensorflow.zeros([n_dense_3])),
            'b_out': tensorflow.Variable(tensorflow.zeros([n_classes]))
        }
        
        weight_dict = {
            'W1': tensorflow.get_variable('W1', [num_features, n_dense_1], initializer=weight_initializer),
            'W2': tensorflow.get_variable('W2', [n_dense_1, n_dense_2], initializer=weight_initializer),
            'W3': tensorflow.get_variable('W3', [n_dense_2, n_dense_3], initializer=weight_initializer),
            'W_out': tensorflow.get_variable('W_out', [n_dense_3, n_classes], initializer=weight_initializer)
        }

        predictions = network(x, weights=weight_dict, biases=bias_dict)
        
        cost = tensorflow.reduce_mean(tensorflow.nn.softmax_cross_entropy_with_logits_v2(logits=predictions, labels=y))
        optimizer = tensorflow.train.GradientDescentOptimizer(learning_rate=learning_rate).minimize(cost)
        
        correct_prediction = tensorflow.equal(tensorflow.argmax(predictions, 1), tensorflow.argmax(y, 1))
        accuracy_pct = tensorflow.reduce_mean(tensorflow.cast(correct_prediction, tensorflow.float32)) * 100
        
        initializer_op = tensorflow.global_variables_initializer()
        history = {'cost':[], 'acc':[]}
        with tensorflow.Session() as session:
            session.run(initializer_op)
            
            print("Training for {} epochs.".format(epochs))
            n_batches = int(mnist.train.num_examples / batch_size)
            start_time = datetime.now()
            progress_time = start_time
            for epoch in range(epochs):
                
                avg_cost = 0.0 # track cost to monitor performance during training
                avg_accuracy_pct = 0.0
                
                for i in range(n_batches):
                    
                    batch_x, batch_y = mnist.train.next_batch(batch_size)
                    
                    # feed batch data to run optimization and fetching cost and accuracy: 
                    _, batch_cost, batch_acc = session.run([optimizer, cost, accuracy_pct], 
                                                           feed_dict={x: batch_x, y: batch_y})
                    
                    # accumulate mean loss and accuracy over epoch: 
                    avg_cost += batch_cost / n_batches
                    avg_accuracy_pct += batch_acc / n_batches
                
                elapsed_time = datetime.now() - progress_time
                if elapsed_time.seconds >=verbose_interval:
                    print("Epoch ", '%03d' % (epoch+1), 
                          ": cost = ", '{:.3f}'.format(avg_cost), 
                          ", accuracy = ", '{:.2f}'.format(avg_accuracy_pct), "%", 
                          sep='')
                    progress_time = datetime.now()
                    
                history['cost'].append(avg_cost)
                history['acc'].append(avg_accuracy_pct)
                
                if avg_accuracy_pct>=99.995:
                    break
            
            elapsed_time = datetime.now() - progress_time
            if elapsed_time.seconds >=1:
                    print("Epoch ", '%03d' % (len(history['cost'])), 
                          ": cost = ", '{:.3f}'.format(history['cost'][-1]), 
                          ", accuracy = ", '{:.2f}'.format(history['acc'][-1]), "%", 
                          sep='')
            
            elapsed_time = datetime.now() - start_time
            print("Training Completed in {} seconds.".format(elapsed_time.seconds))
            
            
            test_cost = cost.eval({x: mnist.test.images, y: mnist.test.labels})
            test_accuracy_pct = accuracy_pct.eval({x: mnist.test.images, y: mnist.test.labels})
            
            print("Test Cost: {:.3f} Test Accuracy: {:.2f}%".format(test_cost, test_accuracy_pct))        
        
        evt = TrainingDoneEvent(TrainingDoneEvent.EVT_WORK_DONE_TYPE, -1)
        wx.PostEvent(self._parent, evt)
                    
if __name__ == '__main__':
    app = wx.App(0)
    frame = MyFrame()
    app.MainLoop()