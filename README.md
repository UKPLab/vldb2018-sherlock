# Sherlock: An Interactive Summarization of Large Text Collections.

In this demo paper, we present a new system for interactive text summarization called Sherlock. The task of automatically producing textual summaries is an important step to understand a collection of multiple topic-related documents. It has many real-world applications in journalism, medicine, and many more. However, none of the existing summarization systems allow users to provide feedback at interactive speed. We therefore integrate a new approximate summarization model into Sherlock that can guarantee interactive speeds even for large text collections to keep the user engaged in the process.

* Online demo: http://sherlock.ukp.informatik.tu-darmstadt.de/
* Video: https://vimeo.com/257601765

If you reuse this software, please use the following citation:

```
@INPROCEEDINGS{PVS:2018a, 
  author = {P.V.S., Avinesh and Hättasch, Benjamin and Özyurt, Orkan and Binnig, Carsten and Meyer, Christian M.}, 
  title = {{Sherlock: A System for Interactive Summarization of Large Text Collections}}, 
  booktitle = {Proceedings of the VLDB Endowment}, 
  pages = {1902--1905}, 
  volume = {11}, 
  number = {12}, 
  month = {August}, 
  year = {2018}, 
  location = {Rio de Janeiro, Brazil}, 
  language = {English}, 
  doi = {10.14778/3229863.3236220}, 
  pdf = {http://www.vldb.org/pvldb/vol11/p1902-p.v.s..pdf}, 
  url = {https://github.com/AIPHES/vldb2018-sherlock/} 
} 
```

**Contact person:**
* Avinesh P.V.S., first_name AT aiphes.tu-darmstadt.de
* Benajamin Haettasch, last_name AT aiphes.tu-darmstadt.de
* https://www.aiphes.tu-darmstadt.de/
* https://www.tu-darmstadt.de/

Don't hesitate to send us an e-mail or report an issue, if something is broken (and it shouldn't be) or if you have further questions.

> This repository contains experimental software and is published for the sole purpose of giving additional background details on the respective publication.


Prerequisites
-------------

* python >= 2.7 (tested with 2.7.6)
* jdk 8
* JAVA_HOME env variable has to be set

## Setting up the Sherlock UI 

1. Install Anaconda

2. Install the requirements
    ```
    pip install -r requirements.txt
    ```
    
3. Install GLPK and CPLEX for PULP (Python Integer Linear Programming Package)
    - Install GLPK
        ```
        sudo apt-get install libglpk-dev
        ```
    - Install CPLEX for Pulp
        ```
        cd ukpsummarizer-be/cplex/cplex/python/2.7/x86-64_linux/
        python setup.py install 
        ```
4. Perl dependencies for ROUGE:
    - LOCAL::LIB 
        ```
        sudo apt-get install liblocal-lib-perl
        ``` 
    - XML::DOM 
        ``` 
        sudo apt-get install libxml-dom-perl
        ```
    - libexpat :
        ``` 
        sudo apt-get install libexpat-dev
        ```
    - libparser
        ``` 
        sudo apt-get install libxml-parser-perl
        ```

5. Bash as the default (source to work):
    ```
    sudo dpkg-reconfigure dash
    ```
    

Build and Run
-------------

Setup the sample data on your system.
```
cp -r data ~/.ukpsummarizer/datasets
```

The result of the build produces `dist/ukpsummarizer-dist-bin.tar` file which should be a standalone bundle.
```
./mvnw clean install
./mvnw -pl ukpsummarizer-server spring-boot:run
```
Alternatively:
```
tar -xvf dist/ukpsummarizer-dist-bin.tar
java -jar ukpsummarizer-server.jar
```  


Preparing Data Directory:
------------------------
1. Create a io directory, perefably in `~/.ukpsummarizer`, which has the following layout:

        +--+cache/
        +--+datasets/
        |  +--+raw/
        |  +--+processed/
        |     +--+DUC2006/
        |     |  +--+D0601A/
        |     |  |  +--+docs/
        |     |  |  +--+docs.parsed/
        |     |  |  +--+summaries/
        |     |  |  +--+summaries.parsed/
        |     |  |  +--+summaries.upperbound/
        |     |  +--+task.json
        |     |  +--+...
        |     +--+ ...
        +--+embeddings/
        |  +--+english/
        |  |  +--+GoogleNews-vectors-negative300.bin
        |  |     +data/
        |  +--+german/
        |     +2014_tudarmstadt_german_50mincount.vec
        ...

2. Download and add the word embeddings into the `~/.ukpsummarizer/embeddings` directory

   Download the Google embeddings (English) from [here](https://drive.google.com/file/d/0B7XkCwpI5KDYNlNUTTlSS21pQmM/)

		 >> mkdir -p summarizer/data/embeddings/english
		 >> mv GoogleNews-vectors-negative300.bin.gz ~/.ukpsummarizer/embeddings/english
		 
   Download the News, Wikipedia embeddings (German) from [here](https://public.ukp.informatik.tu-darmstadt.de/reimers/2014_german_embeddings/2014_tudarmstadt_german_50mincount.vec)
	
		 >> mkdir -p summarizer/data/embeddings/german
		 >> mv 2014_tudarmstadt_german_50mincount.vec ~/.ukpsummarizer/embeddings/german

   Download and install the GloVe embeddings from [here](https://nlp.stanford.edu/projects/glove/)
	
		 >> mkdir -p summarizer/data/embeddings/english/glove
		 >> mv *.txt.w2v ~/.ukpsummarizer/embeddings/english/glove
 

3. Make sure that you have the raw datasets available. Each **raw** dataset needs to be extracted and follow the following directory structure:       

        +--+DUC2006
           +--+docs
           |  +-+D0601A
           |    +-+ many files
           |  +-+D0650E
           +--+models
           |  +-+ many files
           +--+topics.xml


4. Before running the pipeline, you have to preprocess the raw datasets using the `make_data.py` script. 
    
       python ukpsummarizer-be/data_processer/make_data.py -d DUC2006  -p ~/.ukpsummarizer/datasets/raw  -a parse -l english
       python ukpsummarizer-be/data_processer/make_data.py -d DUC2004  -p ~/.ukpsummarizer/datasets/raw  -a parse -l english
       python ukpsummarizer-be/data_processer/make_data.py -d TEST     -p ~/.ukpsummarizer/datasets/raw  -a parse -l english
       python ukpsummarizer-be/data_processer/make_data.py -d DBS      -p ~/.ukpsummarizer/datasets/raw  -a parse -l german

   The results should then be copied into a directory. We recommend using the `--iobasedir` argument to set the directory
 
        +--+cache/
        +--+datasets/
        |  +--+raw/
        |  +--+processed/
        |     +--+DUC2006/
        |     |  +--+D0601A/
        |     |  |  +--+docs/
        |     |  |  +--+docs.parsed/
        |     |  |  +--+summaries/
        |     |  |  +--+summaries.parsed/
        |     |  |  +--+summaries.upperbound/
        |     |  |  +--+task.json
        |     |  +--+...
        |     +--+ ...
        +--+embeddings/
 
Windows setup
=============

Verified by one (1) user.

1. Download and install anaconda2 python 2.7.12 64bit from https://www.continuum.io/downloads#windows , e.g. https://repo.continuum.io/archive/Anaconda2-4.2.0-Windows-x86_64.exe
   * take care that it is NOT python 2.7.13, as that version contains a regression bug which breaks pulp
    
    ``TypeError: LoadLibrary() argument 1 must be string, not unicode``
    
    see http://bugs.python.org/issue29294
    
2. Download + install strawberry perl 64bit. In my case, Strawberry Perl (5.24.0.1-64bit).
3. download + install eclipse neon.2
4. download + instlal eclipse pydev
5. install perl module `XML::DOM`
6. install python modules
    ```
	    pip install -r requirements.txt
    ```

