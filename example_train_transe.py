import config
import models
import tensorflow as tf
import numpy as np
import os
os.environ['CUDA_VISIBLE_DEVICES']='7'
#Input training files from benchmarks/FB15K/ folder.
con = config.Config()
#True: Input test files from the same folder.
con.set_in_path("./masterthesis/nell/")
con.set_test_link_prediction(True)
con.set_test_triple_classification(False)
con.set_work_threads(8)
con.set_train_times(1000)
con.set_nbatches(500)
con.set_alpha(0.001)
con.set_margin(1.0)
con.set_bern(0)
con.set_dimension(100)
con.set_ent_neg_rate(1)
con.set_rel_neg_rate(0)
con.set_opt_method("SGD")
con.set_true_negative_triples(False)
con.set_neg_sample_version(0)

#Models will be exported via tf.Saver() automatically.
con.set_export_files("./masterthesis/nellResults/Complex/corruptFalse/model.vec.tf", 0)
#Model parameters will be exported to json files automatically.
con.set_out_files("./masterthesis/nellResults/Complex/corruptFalse/embedding.vec.json")
#Initialize experimental settings.
con.init()
print("I finished the initalization")
#Set the knowledge embedding model
con.set_model(models.ComplEx)
print("The model was set successfully")
#Train the model.
con.run()
print("Finished Training")
#To test models after training needs "set_test_flag(True)".
#con.test()
#print("Finished Testing")
#con.predict_head_entity(152, 9, 5)
#con.predict_tail_entity(151, 9, 5)
#con.predict_relation(151, 152, 5)
#con.predict_triple(151, 152, 9)
#con.predict_triple(151, 152, 8)
