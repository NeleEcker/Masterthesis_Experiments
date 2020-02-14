#coding:utf-8
import numpy as np
import tensorflow as tf
import os
import time
import datetime
import ctypes
import json
import random

class Config(object):
	'''
	use ctypes to call C functions from python and set essential parameters.
	'''
	def __init__(self):
		base_file = os.path.abspath(os.path.join(os.path.dirname(__file__), '../release/Base.so'))
		self.lib = ctypes.cdll.LoadLibrary(base_file)
		self.lib.sampling.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64]
		#self.lib.samplingWithLinkPrediction.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64*8, ctypes.c_int64*8]
		self.lib.getHeadBatch.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.lib.getTailBatch.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.lib.testHead.argtypes = [ctypes.c_void_p]
		self.lib.testTail.argtypes = [ctypes.c_void_p]
		self.lib.getTestBatch.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.lib.getValidBatch.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.lib.getBestThreshold.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.lib.test_triple_classification.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
		self.test_flag = False
		self.in_path = None
		self.out_path = None
		self.bern = 0
		self.hidden_size = 100
		self.ent_size = self.hidden_size
		self.rel_size = self.hidden_size
		self.train_times = 0
		self.margin = 1.0
		self.nbatches = 100
		self.negative_ent = 1
		self.negative_rel = 0
		self.workThreads = 1
		self.alpha = 0.001
		self.lmbda = 0.000
		self.log_on = 1
		self.exportName = None
		self.importName = None
		self.export_steps = 0
		self.opt_method = "SGD"
		self.optimizer = None
		self.test_link_prediction = False
		self.test_triple_classification = False
		self.early_stopping = None # It expects a tuple of the following: (patience, min_delta)

	def init_link_prediction(self):
		r'''
		import essential files and set essential interfaces for link prediction
		'''
		self.lib.importTestFiles()
		self.lib.importTypeFiles()
		self.test_h = np.zeros(self.lib.getEntityTotal(), dtype = np.int64)
		self.test_t = np.zeros(self.lib.getEntityTotal(), dtype = np.int64)
		self.test_r = np.zeros(self.lib.getEntityTotal(), dtype = np.int64)
		self.test_h_addr = self.test_h.__array_interface__['data'][0]
		self.test_t_addr = self.test_t.__array_interface__['data'][0]
		self.test_r_addr = self.test_r.__array_interface__['data'][0]

	def init_triple_classification(self):
		r'''
		import essential files and set essential interfaces for triple classification
		'''
		self.lib.importTestFiles()
		self.lib.importTypeFiles()

		self.test_pos_h = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_pos_t = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_pos_r = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_neg_h = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_neg_t = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_neg_r = np.zeros(self.lib.getTestTotal(), dtype = np.int64)
		self.test_pos_h_addr = self.test_pos_h.__array_interface__['data'][0]
		self.test_pos_t_addr = self.test_pos_t.__array_interface__['data'][0]
		self.test_pos_r_addr = self.test_pos_r.__array_interface__['data'][0]
		self.test_neg_h_addr = self.test_neg_h.__array_interface__['data'][0]
		self.test_neg_t_addr = self.test_neg_t.__array_interface__['data'][0]
		self.test_neg_r_addr = self.test_neg_r.__array_interface__['data'][0]

		self.valid_pos_h = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_pos_t = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_pos_r = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_neg_h = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_neg_t = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_neg_r = np.zeros(self.lib.getValidTotal(), dtype = np.int64)
		self.valid_pos_h_addr = self.valid_pos_h.__array_interface__['data'][0]
		self.valid_pos_t_addr = self.valid_pos_t.__array_interface__['data'][0]
		self.valid_pos_r_addr = self.valid_pos_r.__array_interface__['data'][0]
		self.valid_neg_h_addr = self.valid_neg_h.__array_interface__['data'][0]
		self.valid_neg_t_addr = self.valid_neg_t.__array_interface__['data'][0]
		self.valid_neg_r_addr = self.valid_neg_r.__array_interface__['data'][0]
		self.relThresh = np.zeros(self.lib.getRelationTotal(), dtype = np.float32)
		self.relThresh_addr = self.relThresh.__array_interface__['data'][0]

	# prepare for train and test
	def init(self):
		self.trainModel = None
		if self.in_path != None:
			self.lib.setInPath(ctypes.create_string_buffer(self.in_path.encode(), len(self.in_path) * 2))
			self.lib.setBern(self.bern)
			self.lib.setTrueNegativeSamplesFlag(self.true_negative_triples)
			self.lib.setNegativeSampleVersion(self.neg_sample_version)
			print(self.neg_sample_version)
			self.lib.setWorkThreads(self.workThreads)
			self.lib.randReset()
			self.lib.importTrainFiles()
			self.relTotal = self.lib.getRelationTotal()
			self.entTotal = self.lib.getEntityTotal()
			self.trainTotal = self.lib.getTrainTotal()
			self.testTotal = self.lib.getTestTotal()
			self.validTotal = self.lib.getValidTotal()
			self.batch_size = int(self.lib.getTrainTotal() / self.nbatches)
			self.batch_seq_size = self.batch_size * (1 + self.negative_ent + self.negative_rel)
			self.batch_h = np.zeros(self.batch_size * (1 + self.negative_ent + self.negative_rel), dtype = np.int64)
			self.batch_t = np.zeros(self.batch_size * (1 + self.negative_ent + self.negative_rel), dtype = np.int64)
			self.batch_r = np.zeros(self.batch_size * (1 + self.negative_ent + self.negative_rel), dtype = np.int64)
			self.batch_y = np.zeros(self.batch_size * (1 + self.negative_ent + self.negative_rel), dtype = np.float32)
			self.batch_h_addr = self.batch_h.__array_interface__['data'][0]
			self.batch_t_addr = self.batch_t.__array_interface__['data'][0]
			self.batch_r_addr = self.batch_r.__array_interface__['data'][0]
			self.batch_y_addr = self.batch_y.__array_interface__['data'][0]
		if self.test_link_prediction:
			self.init_link_prediction()
		if self.test_triple_classification:
			self.init_triple_classification()
		if self.true_negative_triples:
			self.lib.importNegativeSamples()

	def get_ent_total(self):
		return self.entTotal

	def get_rel_total(self):
		return self.relTotal

	def set_lmbda(self, lmbda):
		self.lmbda = lmbda

	def set_optimizer(self, optimizer):
		self.optimizer = optimizer

	def set_opt_method(self, method):
		self.opt_method = method

	def set_test_link_prediction(self, flag):
		self.test_link_prediction = flag

	def set_test_triple_classification(self, flag):
		self.test_triple_classification = flag

	def set_true_negative_triples(self, flag):
		self.true_negative_triples = flag

	def set_neg_sample_version(self, flag):
		self.neg_sample_version = flag

	def set_log_on(self, flag):
		self.log_on = flag

	def set_alpha(self, alpha):
		self.alpha = alpha

	def set_in_path(self, path):
		self.in_path = path

	def set_out_files(self, path):
		self.out_path = path

	def set_bern(self, bern):
		self.bern = bern

	def set_dimension(self, dim):
		self.hidden_size = dim
		self.ent_size = dim
		self.rel_size = dim

	def set_ent_dimension(self, dim):
		self.ent_size = dim

	def set_rel_dimension(self, dim):
		self.rel_size = dim

	def set_train_times(self, times):
		self.train_times = times

	def set_nbatches(self, nbatches):
		self.nbatches = nbatches

	def set_margin(self, margin):
		self.margin = margin

	def set_work_threads(self, threads):
		self.workThreads = threads

	def set_ent_neg_rate(self, rate):
		self.negative_ent = rate

	def set_rel_neg_rate(self, rate):
		self.negative_rel = rate

	def set_import_files(self, path):
		self.importName = path

	def set_export_files(self, path, steps = 0):
		self.exportName = path
		self.export_steps = steps

	def set_export_steps(self, steps):
		self.export_steps = steps

	def set_early_stopping(self, early_stopping):
		self.early_stopping = early_stopping

	# call C function for sampling
	def sampling(self, epochNumber):
		self.lib.sampling(self.batch_h_addr, self.batch_t_addr, self.batch_r_addr, self.batch_y_addr, self.batch_size, self.negative_ent, self.negative_rel, 10 , epochNumber)

	#def samplingWithLinkPrediction(self, epochNumber, randomNumbers, corruptedHeads):
	#	self.lib.samplingWithLinkPrediction(self.batch_h_addr, self.batch_t_addr, self.batch_r_addr, self.batch_y_addr, self.batch_size, self.negative_ent, self.negative_rel, 10 , epochNumber, randomNumbers, corruptedHeads)

	# save model
	def save_tensorflow(self):
		with self.graph.as_default():
			with self.sess.as_default():
				self.saver.save(self.sess, self.exportName)
	# restore model
	def restore_tensorflow(self):
		with self.graph.as_default():
			with self.sess.as_default():
				self.saver.restore(self.sess, self.importName)


	def export_variables(self, path = None):
		with self.graph.as_default():
			with self.sess.as_default():
				if path == None:
					self.saver.save(self.sess, self.exportName)
				else:
					self.saver.save(self.sess, path)

	def import_variables(self, path = None):
		with self.graph.as_default():
			with self.sess.as_default():
				if path == None:
					self.saver.restore(self.sess, self.importName)
				else:
					self.saver.restore(self.sess, path)

	def get_parameter_lists(self):
		return self.trainModel.parameter_lists

	def get_parameters_by_name(self, var_name):
		with self.graph.as_default():
			with self.sess.as_default():
				if var_name in self.trainModel.parameter_lists:
					return self.sess.run(self.trainModel.parameter_lists[var_name])
				else:
					return None

	def get_parameters(self, mode = "numpy"):
		res = {}
		lists = self.get_parameter_lists()
		for var_name in lists:
			if mode == "numpy":
				res[var_name] = self.get_parameters_by_name(var_name)
			else:
				res[var_name] = self.get_parameters_by_name(var_name).tolist()
		return res

	def save_parameters(self, path = None):
		if path == None:
			path = self.out_path
		f = open(path, "w")
		f.write(json.dumps(self.get_parameters("list")))
		f.close()

	def set_parameters_by_name(self, var_name, tensor):
		with self.graph.as_default():
			with self.sess.as_default():
				if var_name in self.trainModel.parameter_lists:
					self.trainModel.parameter_lists[var_name].assign(tensor).eval()

	def set_parameters(self, lists):
		for i in lists:
			self.set_parameters_by_name(i, lists[i])

	def set_model(self, model):
		self.model = model
		self.graph = tf.Graph()
		tf.reset_default_graph()
		with self.graph.as_default():
			config = tf.ConfigProto()
			config.graph_options.optimizer_options.global_jit_level = tf.OptimizerOptions.ON_1
			self.sess = tf.Session(config=config)
			with self.sess.as_default():
				initializer = tf.contrib.layers.xavier_initializer(uniform = True)
				with tf.variable_scope("model", reuse=None, initializer = initializer):
					self.trainModel = self.model(config = self)
					if self.optimizer != None:
						pass
					elif self.opt_method == "Adagrad" or self.opt_method == "adagrad":
						self.optimizer = tf.train.AdagradOptimizer(learning_rate = self.alpha, initial_accumulator_value=1e-20)
					elif self.opt_method == "Adadelta" or self.opt_method == "adadelta":
						self.optimizer = tf.train.AdadeltaOptimizer(self.alpha)
					elif self.opt_method == "Adam" or self.opt_method == "adam":
						self.optimizer = tf.train.AdamOptimizer(self.alpha)
					else:
						self.optimizer = tf.train.GradientDescentOptimizer(self.alpha)
					grads_and_vars = self.optimizer.compute_gradients(self.trainModel.loss)
					self.train_op = self.optimizer.apply_gradients(grads_and_vars)
				self.saver = tf.train.Saver()
				self.sess.run(tf.global_variables_initializer())

	def train_step(self, batch_h, batch_t, batch_r, batch_y):
		feed_dict = {
			self.trainModel.batch_h: batch_h,
			self.trainModel.batch_t: batch_t,
			self.trainModel.batch_r: batch_r,
			self.trainModel.batch_y: batch_y
		}
		_, loss = self.sess.run([self.train_op, self.trainModel.loss], feed_dict)
		return loss

	def test_step(self, test_h, test_t, test_r):
		feed_dict = {
			self.trainModel.predict_h: test_h,
			self.trainModel.predict_t: test_t,
			self.trainModel.predict_r: test_r,
		}
		predict = self.sess.run(self.trainModel.predict, feed_dict)
		return predict

	def run(self):
		with self.graph.as_default():
			with self.sess.as_default():
				if self.importName != None:
					self.restore_tensorflow()
				if self.early_stopping is not None:
					patience, min_delta = self.early_stopping
					best_loss = np.finfo('float32').max
					wait_steps = 0
				file = open("./masterthesis/nellResults/TransE/trueFalseV3correct/generalTimes.txt", "w+");
				if self.neg_sample_version == 6:
					self.set_neg_sample_version(0)
					self.lib.setNegativeSampleVersion(self.neg_sample_version)
					for times in range(self.train_times):
						if times == 0.8*self.train_times:
							self.set_neg_sample_version(4)
							self.lib.setNegativeSampleVersion(self.neg_sample_version)
						loss = 0.0
						t_init = time.time()
						for batch in range(self.nbatches):
							self.sampling(times)
							loss += self.train_step(self.batch_h, self.batch_t, self.batch_r, self.batch_y)
						t_end = time.time()
						if self.log_on:
							print('Epoch: {}, loss: {}, time: {}'.format(times, loss, (t_end - t_init)))
							line = ('{},{},{}\n'.format(times, loss, (t_end - t_init)))
							file.write(line)
						if self.exportName != None and (self.export_steps!=0 and times % self.export_steps == 0):
							self.save_tensorflow()
						if self.early_stopping is not None:
							if loss + min_delta < best_loss:
								best_loss = loss
								wait_steps = 0
							elif wait_steps < patience:
								wait_steps += 1
							else:
								print('Early stopping. Losses have not been improved enough in {} times'.format(patience))
								break

				else:
					for times in range(self.train_times):
						#self.lib.callPython()
						#print(1)
						loss = 0.0
						t_init = time.time()
						for batch in range(self.nbatches):
							#if(self.negSampleVersion == 5):
							#	randomNumbers = self.getRand(self.workThreads)
							#	corruptedHeads = self.getPredictionForCorruption(randomNumbers)
								#self.samplingWithLinkPrediction(times, randomNumbers, corruptedHeads)
							#else :
							self.sampling(times)
							loss += self.train_step(self.batch_h, self.batch_t, self.batch_r, self.batch_y)
						t_end = time.time()
						if self.log_on:
							print('Epoch: {}, loss: {}, time: {}'.format(times, loss, (t_end - t_init)))
							line = ('{},{},{}\n'.format(times, loss, (t_end - t_init)))
							file.write(line)
						if self.exportName != None and (self.export_steps!=0 and times % self.export_steps == 0):
							self.save_tensorflow()
						if self.early_stopping is not None:
							if loss + min_delta < best_loss:
								best_loss = loss
								wait_steps = 0
							elif wait_steps < patience:
								wait_steps += 1
							else:
								print('Early stopping. Losses have not been improved enough in {} times'.format(patience))
								break
				file.close()
				if self.exportName != None:
					self.save_tensorflow()
				if self.out_path != None:
					self.save_parameters(self.out_path)

	def getRand(self, nThreads):
		randomNumbers = []
		for i in range(nThreads):
			rand = random.randint(0, self.batch_size)
			randomNumbers.append(rand)
		return randomNumbers

	def getPredictionForCorruption(self, aRandomNumbers):
		corruptedHeads = []
		for i in aRandomNumbers:
			tail = self.batch_t[i]
			relation = self.batch_r[i]
			res = self.predict_head_entity(tail, relation, 2)
			if(res[0] == self.batch_h[i]):
				corruptedHeads.append(res[1])
			else:
				corruptedHeads.append(res[0])
		return corruptedHeads

	def test(self):
		with self.graph.as_default():
			with self.sess.as_default():
				if self.importName != None:
					self.restore_tensorflow()
				if self.test_link_prediction:
					total = self.lib.getTestTotal()
					for times in range(total):
						self.lib.getHeadBatch(self.test_h_addr, self.test_t_addr, self.test_r_addr)
						res = self.test_step(self.test_h, self.test_t, self.test_r)
						self.lib.testHead(res.__array_interface__['data'][0])

						self.lib.getTailBatch(self.test_h_addr, self.test_t_addr, self.test_r_addr)
						res = self.test_step(self.test_h, self.test_t, self.test_r)
						self.lib.testTail(res.__array_interface__['data'][0])
						if times % 100 == 0:
							print(times)
					self.lib.test_link_prediction()
				if self.test_triple_classification:
					self.lib.getValidBatch(self.valid_pos_h_addr, self.valid_pos_t_addr, self.valid_pos_r_addr, self.valid_neg_h_addr, self.valid_neg_t_addr, self.valid_neg_r_addr)
					res_pos = self.test_step(self.valid_pos_h, self.valid_pos_t, self.valid_pos_r)
					res_neg = self.test_step(self.valid_neg_h, self.valid_neg_t, self.valid_neg_r)
					self.lib.getBestThreshold(self.relThresh_addr, res_pos.__array_interface__['data'][0], res_neg.__array_interface__['data'][0])

					self.lib.getTestBatch(self.test_pos_h_addr, self.test_pos_t_addr, self.test_pos_r_addr, self.test_neg_h_addr, self.test_neg_t_addr, self.test_neg_r_addr)

					res_pos = self.test_step(self.test_pos_h, self.test_pos_t, self.test_pos_r)
					res_neg = self.test_step(self.test_neg_h, self.test_neg_t, self.test_neg_r)
					self.lib.test_triple_classification(self.relThresh_addr, res_pos.__array_interface__['data'][0], res_neg.__array_interface__['data'][0])

	def predict_head_entity(self, t, r, k):
		r'''This mothod predicts the top k head entities given tail entity and relation.

		Args:
			t (int): tail entity id
			r (int): relation id
			k (int): top k head entities

		Returns:
			list: k possible head entity ids
		'''
		self.init_link_prediction()
		if self.importName != None:
			self.restore_tensorflow()
		test_h = np.array(range(self.entTotal))
		test_r = np.array([r] * self.entTotal)
		test_t = np.array([t] * self.entTotal)
		res = self.test_step(test_h, test_t, test_r).reshape(-1).argsort()[:k]
		print(res)
		return res

	def predict_tail_entity(self, h, r, k):
		r'''This mothod predicts the top k tail entities given head entity and relation.

		Args:
			h (int): head entity id
			r (int): relation id
			k (int): top k tail entities

		Returns:
			list: k possible tail entity ids
		'''
		self.init_link_prediction()
		if self.importName != None:
			self.restore_tensorflow()
		test_h = np.array([h] * self.entTotal)
		test_r = np.array([r] * self.entTotal)
		test_t = np.array(range(self.entTotal))
		res = self.test_step(test_h, test_t, test_r).reshape(-1).argsort()[:k]
		print(res)
		return res

	def predict_relation(self, h, t, k):
		r'''This methods predict the relation id given head entity and tail entity.

		Args:
			h (int): head entity id
			t (int): tail entity id
			k (int): top k relations

		Returns:
			list: k possible relation ids
		'''
		self.init_link_prediction()
		if self.importName != None:
			self.restore_tensorflow()
		test_h = np.array([h] * self.relTotal)
		test_r = np.array(range(self.relTotal))
		test_t = np.array([t] * self.relTotal)
		res = self.test_step(test_h, test_t, test_r).reshape(-1).argsort()[:k]
		print(res)
		return res

	def predict_triple(self, h, t, r, thresh = None):
		r'''This method tells you whether the given triple (h, t, r) is correct of wrong

		Args:
			h (int): head entity id
			t (int): tail entity id
			r (int): relation id
			thresh (fload): threshold for the triple
		'''
		self.init_triple_classification()
		if self.importName != None:
			self.restore_tensorflow()
		res = self.test_step(np.array([h]), np.array([t]), np.array([r]))
		if thresh != None:
			if res < thresh:
				print("triple (%d,%d,%d) is correct" % (h, t, r))
			else:
				print("triple (%d,%d,%d) is wrong" % (h, t, r))
			return
		self.lib.getValidBatch(self.valid_pos_h_addr, self.valid_pos_t_addr, self.valid_pos_r_addr, self.valid_neg_h_addr, self.valid_neg_t_addr, self.valid_neg_r_addr)
		res_pos = self.test_step(self.valid_pos_h, self.valid_pos_t, self.valid_pos_r)
		res_neg = self.test_step(self.valid_neg_h, self.valid_neg_t, self.valid_neg_r)
		self.lib.getBestThreshold(self.relThresh_addr, res_pos.__array_interface__['data'][0], res_neg.__array_interface__['data'][0])
		if res < self.relThresh[r]:
			print("triple (%d,%d,%d) is correct" % (h, t, r))
		else:
			print("triple (%d,%d,%d) is wrong" % (h, t, r))
