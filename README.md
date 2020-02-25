 # First steps
 
 To setup the OpenKE framework please follow the instructions in the file READMEOpenKE.md. 
 Once the setup is finished the following steps regarding the training and testing can be performed to use the specific features that have been implemented in the Masterthesis.
 
 # Training

The training can be started by executing the Python file example_train_transe.py.
Different adaptions need to be made. First multiple paths need to be adapted. This
includes the path to the data set that shall be used for the training. 

Furthermore,
a path need to be specified to store the embedding and the model. The path also
needs to be adapted in the file Config.py. This enables that the training times and
loss function values of each epoch are stored.

After all paths have been adapted different settings can be done in the first
mentioned file:


• set_neg_sample_version: This parameter determines which approach shall
be used to pick the negative fact. 0 is the corruption approach. 1 to 4 are the
proposed approaches 1 to 4.


• set_opt_method: This determines which optimization algorithm shall be
used. The default is SGD but it can be changed to Adagrad, Adadelta and
Adam.


• set_ent_neg_rate: This determines how many negative facts per positive fact
shall be picked.

• set_model: This parameter determines which model shall be trained. This
can be for example TransE, TransD, ComplEx or ANALOGY.

After having set the parameters, the training can be started.




# Testing


The testing can be performed running the Python file example_test_transe.py. Some
paths need to be adapted in this file as well. The path to the data set needs to be set
as in the training file. Furthermore, the path to the model that shall be tested need
to be set. It has to be the same path that has been specified in the training file to
store the model. Of all parameters that were set in the training file only the model
is of interest for the testing. It can be set using set_model as well.

The testing results are printed into the console after the testing has been finished.
This is followed by the link prediction results. It is implemented that the
result is printed into the C++ console so make sure that the Python console can also
show the C++ output.

At the end of the testing program two facts are set to perform the link prediction.
 They can also be adapted to test a different fact.
