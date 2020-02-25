import config
import models
import tensorflow as tf
import numpy as np
import json
import os
import datetime
os.environ['CUDA_VISIBLE_DEVICES']='7'
# (1) Set import files and OpenKE will automatically load models via tf.Saver().
con = config.Config()
#con.set_in_path("./masterthesis/nell/")
con.set_in_path("./masterthesis/DBPedia500KNew/")
con.set_test_link_prediction(True)
#con.set_test_triple_classification(True)
con.set_work_threads(8)
con.set_dimension(100)
con.set_true_negative_triples(True)
con.set_neg_sample_version(4)
con.set_import_files("./masterthesis/dbpediaResultsSampleNew/TransE/trueFalseV3correct/model.vec.tf")
#con.set_import_files("./masterthesis/nellResults/TransE/trueFalseV3correct/model.vec.tf")
con.init()
con.set_model(models.TransE)
con.test()
print("Testing done")
print(datetime.datetime.now())
#227189 233532 83
#Good sample dbpedia
con.predict_head_entity(271003, 6, 10)
con.predict_tail_entity(398710, 6, 10)
#good sample nell
#con.predict_head_entity(128772, 225, 10)
#con.predict_tail_entity(50728, 255, 10)