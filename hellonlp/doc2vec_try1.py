#!/usr/bin/env python3

# sudo pip3 install --upgrade pip
# sudo pip3 install gensim nltk
from gensim.models.doc2vec import Doc2Vec, TaggedDocument
from nltk.tokenize import word_tokenize
import nltk
import argparse
import os
import os.path
import time
import multiprocessing
import sys

train_set="TITLES_US_1M.txt"

class doc2vec_driver:
    def __init__(self):
        self.max_epochs = 200
        self.vec_size = 50
        self.alpha = 0.025
        self.verbose_after_loops = 10
        self.verbose_after_seconds = 10
        self.resume_mode = True
        self.alpha_delta = 0.0001
        self.min_count=5
        self.model_file_name = os.getcwd() + "/d2v.model"
        self.dm=1 # If dm=1, ‘distributed memory’ (PV-DM) is used. Otherwise, distributed bag of words (PV-DBOW) is employed.
        
    def summary(self):
        print("max_epochs:\t"+str(self.max_epochs))
        print("vec_size:\t"+str(self.vec_size))
        print("alpha:\t\t"+str(self.alpha))
        print("alpha_delta:\t"+str(self.alpha_delta))
        print("resume_mode:\t"+str(self.resume_mode))
        print("model_file_name:\t"+str(self.model_file_name))
        
    def train_doc2vec(self, tagged_data):            
        if self.resume_mode and os.path.isfile(self.model_file_name):
            print("resume training from " + self.model_file_name)
            model = Doc2Vec.load(self.model_file_name)
        else:
            model = Doc2Vec(vector_size=self.vec_size,
                            alpha=self.alpha, 
#                             min_alpha=self.alpha - self.alpha_delta,
                            min_count=self.min_count,
                            workers=multiprocessing.cpu_count()*2, 
                            dm =self.dm)
            model.build_vocab(tagged_data)
    
        start_time = time.time()
        
        for epoch in range(self.max_epochs):
            epoch_start_time = time.time()
            model.train(tagged_data,
                        total_examples=model.corpus_count,
                        epochs=model.iter)
            # decrease the learning rate
            model.alpha -= self.alpha_delta
            if model.alpha < self.alpha_delta:
                print("alpha too small {} at epoch {}".format(model.alpha, epoch))
                break
            # fix the learning rate, no decay
            model.min_alpha = model.alpha
            epoch_used_seconds = time.time() - epoch_start_time
            if epoch_used_seconds > self.verbose_after_seconds or self.verbose_after_loops==1 or epoch%self.verbose_after_loops == self.verbose_after_loops-1:
                print('iteration {} used {} seconds'.format(epoch, epoch_used_seconds))
        
        model.save(self.model_file_name)
        print("Doc2Vec model saved to " + self.model_file_name)
    
    def file_to_array(self, filepath):
        lines = []
        with open(filepath) as fp:  
            for line in fp:
                lines.append(line)
        return lines
    
    def array_to_tagged_data(self, lines):  
        tagged_data = [TaggedDocument(words=word_tokenize(_d.lower()), tags=[str(i)]) for i, _d in enumerate(lines)]
        return tagged_data
    
    def test_model(self, doc_collection, test_doc):
        model= Doc2Vec.load(self.model_file_name)
        model.delete_temporary_training_data(keep_doctags_vectors=True, keep_inference=True)
        
        test_data = word_tokenize(test_doc.lower())
        new_vector = model.infer_vector(test_data)
        
        similar_doc = model.docvecs.most_similar([new_vector])     
        print("most_similar documents to " + test_doc)

        for tp in similar_doc:
            doc_no = int(tp[0])
            if doc_collection[doc_no].endswith("\n"):
                print(tp[0] + "\t" + doc_collection[doc_no], end="")        
            else:
                print(tp[0] + "\t" + doc_collection[doc_no])
    
    def train_dummy_model(self):
        corpus_data = ["dummy line 1.",
                "dummy line 2.",
                "third line placeholder.",
                "I love machine learning. Its awesome.",
                "I love coding in python.",
                "I love coding in java.",
                "I love coding in javascript.",
                "They are awesome.",
                "The weather is good today.",
                "The weather is nice today.",
                "The weather is aweful today.",
                "I love building chatbots.",
                "they chat amagingly well."]
        
        tagged_data = self.array_to_tagged_data(corpus_data)
        self.verbose_after_loops = 50
        self.train_doc2vec(tagged_data)
        test_doc = "I love python."
        self.test_model(corpus_data, test_doc)
                
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='doc2vec')
    parser.add_argument("-t", "--train", action='store_true', default=False,
                    help="Train doc2vec model.")
    parser.add_argument("-i", "--infer", action='store_true',
                    help="infer_vector doc2vec model.")
    parser.add_argument("--download_nltk", type=str,
                    help="specify which nltk resource to download.")
    parser.add_argument("-f", "--model_file_name", default="d2v.model", type=str,
                    help="model_file_name.")
    parser.add_argument("-c", "--cwd", default=os.getcwd(), type=str,
                    help="cwd.")
    parser.add_argument("-e", "--epochs", default=1, type=int,
                    help="epochs.")
    parser.add_argument("-d", "--docs", default=train_set, type=str,
                    help="docs to train.")

    args = parser.parse_args()

    if args.download_nltk is not None:
        nltk.download(args.download_nltk)

    train_set = args.docs
    used_times={}
    dd = doc2vec_driver()
    dd.model_file_name = args.cwd + "/" + args.model_file_name
    lines = dd.file_to_array(train_set)
    
    if args.train:
        tagged_data = dd.array_to_tagged_data(lines)
        dd.max_epochs = args.epochs
        dd.train_doc2vec(tagged_data)
        
    dd.test_model(lines, "new ipod")
    dd.summary()
    sys.exit(0)
    
    
    if args.train:
        start_time = time.time()
        train_doc2vec(train_set, args.epochs, model_file_name)
        used_times['train_doc2vec'] = time.time() - start_time
    
    start_time = time.time()
    test_doc = "whirlpool oven"
    try_model(model_file_name, test_doc)
    used_times['try_model'] = time.time() - start_time
    
    for k,v in used_times.items():
        print('{} used {} seconds'.format(k, v))

