# usage:
"""
python synonym.py -w panda
synsets of panda: [Synset('giant_panda.n.01'), Synset('lesser_panda.n.01')]
synonyms of panda: {'red_panda', 'bear_cat', 'Ailuropoda_melanoleuca', 'panda', 'lesser_panda', 'Ailurus_fulgens', 'coon_bear', 'panda_bear', 'giant_panda', 'cat_bear'}
antonyms of panda: set()
"""
# [nltk_data] Error loading wordnet: <urlopen error [SSL:
# [nltk_data]     CERTIFICATE_VERIFY_FAILED] certificate verify failed
# [nltk_data]     (_ssl.c:777)>
# /Applications/Python 3.6/Install Certificates.command

import nltk
from nltk.corpus import wordnet
import argparse
import sys

def find_related_words(word):
  synonyms = []
  antonyms = []

  syns = wordnet.synsets(word)
  print("synsets of " + word, end=": ")
  print(syns)

  for syn in syns:
    for l in syn.lemmas():
      synonyms.append(l.name())
      if l.antonyms():
        antonyms.append(l.antonyms()[0].name())
  print("synonyms of " + word, end=": ")
  print(set(synonyms))
  print("antonyms of " + word, end=": ")
  print(set(antonyms))

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Accepted arguments')
  parser.add_argument('-w', '--word', help='Word to find synonyms and antonyms')
  parser.add_argument('-i', '--init', help='Download wordnet', action='store_true', required=False)
  args = vars(parser.parse_args())

  if args['word'] is None:
    parser.print_help(sys.stderr)
    sys.exit(1)

  if args['init']:
    nltk.download('wordnet')

  find_related_words(args['word'])
