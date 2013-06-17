# distutils: language = c++
# distutils: include_dirs = . ../src
# distutils: extra_objects = ../bin/libstan.a ../bin/libstanc.a
from libcpp.string cimport string
from cpython cimport bool
import sys
import random

from scipy import weave
from scipy.weave import converters

cdef extern from "stanc.hpp":
    string stanc(string model_code,
                 string model_name,
                 error_buf) except +

def stan_model(model_code= "anon_model",
               model_name = '',
               stanc_ret = None,
               boost_lib = None,
               eigen_lib = None,
               save_dso = True,
               verbose = False,
               ): #...):
    return stanc(model_code, "_"+model_name, sys.stderr)

def stan(file,
         sample_file, # the file to which the samples are written
         model_name = "anon_model",
         model_code = '',
         fit = None,
         data = list(),
         pars = None,
         chains = 4, iter = 2000,
         warmup = 1000, #floor(iter / 2.0),
         thin = 1,
         init = "random",
         seed = random.randint(0, sys.maxint), # sample.int(.Machine$integer.max, 1),
         save_dso = True,
         verbose = False, #...,
         boost_lib = None,
         eigen_lib = None):
    pass

cpdef extract(model_code, model_name):
    pass

def samples():
    pass

def test():
    model_code = """
    data {
      int<lower=0> J; // number of schools 
      real y[J]; // estimated treatment effects
      real<lower=0> sigma[J]; // s.e. of effect estimates 
    }
    parameters {
      real mu; 
      real<lower=0> tau;
      real eta[J];
    }
    transformed parameters {
      real theta[J];
      for (j in 1:J)
        theta[j] <- mu + tau * eta[j];
    }
    model {
      eta ~ normal(0, 1);
      y ~ normal(theta, sigma);
    }
    """
    return stan_model(model_code, "8schools")
    
def test2():
    code = test()
    weave.inline(code,
                 [],
                 libraries=['boost', 'eigen'],
                 library_dirs=['../lib','../bin'],
                 type_converters=converters.blitz,
                 compiler='gcc')
    
