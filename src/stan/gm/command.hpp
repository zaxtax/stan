#ifndef __STAN__GM__COMMAND_HPP__
#define __STAN__GM__COMMAND_HPP__

//#include <cmath>
//#include <cstddef>
//#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
//#include <iomanip>
//#include <iostream>
#include <fstream>
#include <sstream>
//#include <vector>
#include <boost/random/additive_combine.hpp> // L'Ecuyer RNG
//#include <boost/random/mersenne_twister.hpp>
//#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <stan/version.hpp>
#include <stan/io/cmd_line.hpp>
#include <stan/io/dump.hpp>
//#include <stan/mcmc/adaptive_sampler.hpp>
#include <stan/mcmc/adaptive_hmc.hpp>
//#include <stan/mcmc/hmc.hpp>
#include <stan/mcmc/nuts.hpp>
#include <stan/mcmc/nuts_diag.hpp>
#include <stan/mcmc/nuts_nondiag.hpp>
#include <stan/mcmc/nuts_massgiven.hpp>
//#include <stan/model/prob_grad_ad.hpp>
//#include <stan/model/prob_grad.hpp>
//#include <stan/mcmc/sampler.hpp>
#include <stan/optimization/newton.hpp>

namespace stan {

  namespace gm {
    
    namespace {
      struct contour_info {
	contour_info() 
	  : has_contour(false),
	    idx0(0), min0(0), max0(0),
	    idx1(0), min1(0), max1(0),
	    n(0) {}

	bool has_contour;
	int idx0;
	double min0, max0;
	int idx1;
	double min1, max1;
	int n;
      
	bool populate(const std::string& contour_string) {
	  has_contour = false;
	  if (contour_string.find("(") != 0)
	    return false;
	  if (contour_string.find(")") != contour_string.size()-1)
	    return false;
	  std::stringstream ss(contour_string);
	  if (ss.get() && !(ss >> idx0)) // pop '(', read idx0
	    return false;
	  if (ss.get() && !(ss >> min0)) // pop ',', read min0
	    return false;
	  if (ss.get() && !(ss >> max0)) // pop ',', read max0
	    return false;
	  if (ss.get() && !(ss >> idx1)) // pop ',', read idx1
	    return false;
	  if (ss.get() && !(ss >> min1)) // pop ',', read min1
	    return false;
	  if (ss.get() && !(ss >> max1)) // pop ',', read max1
	    return false;
	  if (ss.get() == ',') {         // if ',', read n, then ')'
	    if (!(ss >> n) || (ss.get() < 0))
	      return false;
	  } else {                       // else set n to default
	    n = 101;
	  }
	  if (ss.get() > -1)             // final check to see if the end was reached
	    return false;

	  has_contour = true;
	  return true;
	}

	bool is_valid() {
	  using std::isinf;
	  if (has_contour == false)
	    return false;
	  if (idx0 < 0 || idx1 < 0 || n < 1)
	    return false;
	  if (!(min0 < max0) || !(min1 < max1))
	    return false;
	  if (isinf(min0) || isinf(max0) || isinf(min1) || isinf(max1))
	    return false;
	  return true;
	}
      
	friend std::ostream& operator<<(std::ostream& os, const contour_info& contour);
      };
    
      std::ostream& operator<<(std::ostream& os, const contour_info& contour) {
	os << "contour_info:" << std::endl
	   << "  has_contour: " << (contour.has_contour?"true":"false") << std::endl;
	if (contour.has_contour == false)
	  return os;

	os << "  index 0:     " << contour.idx0 << " (" << contour.min0 << ", " << contour.max0 << ")" << std::endl
	   << "  index 1:     " << contour.idx1 << " (" << contour.min1 << ", " << contour.max1 << ")" << std::endl
	   << "  n:           " << contour.n << std::endl;
	return os;
      }
    }
        
    void print_nuts_help(std::string cmd) {
      using stan::io::print_help_option;

      std::cout << std::endl;
      std::cout << "Compiled Stan Graphical Model Command" << std::endl;
      std::cout << std::endl;

      std::cout << "USAGE:  " << cmd << " [options]" << std::endl;
      std::cout << std::endl;

      std::cout << "OPTIONS:" << std::endl;
      std::cout << std::endl;

      print_help_option(&std::cout,
                        "help","",
                        "Display this information");

      print_help_option(&std::cout,
                        "data","file",
                        "Read data from specified dump-format file",
                        "required if model declares data");
      
      print_help_option(&std::cout,
                        "init","file",
                        "Use initial values from specified file or zero values if <file>=0",
                        "default is random initialization");

      print_help_option(&std::cout,
                        "samples","file",
                        "File into which samples are written",
                        "default = samples.csv");

      print_help_option(&std::cout,
                        "append_samples","",
                        "Append samples to existing file if it exists",
                        "does not write header in append mode");

      print_help_option(&std::cout,
                        "seed","int",
                        "Random number generation seed",
                        "default = randomly generated from time");

      print_help_option(&std::cout,
                        "chain_id","int",
                        "Markov chain identifier",
                        "default = 1");

      print_help_option(&std::cout,
                        "iter","+int",
                        "Total number of iterations, including warmup",
                        "default = 2000");

      print_help_option(&std::cout,
                        "warmup","+int",
                        "Discard the specified number of initial samples",
                        "default = iter / 2");

      print_help_option(&std::cout,
                        "thin","+int",
                        "Period between saved samples after warm up",
                        "default = max(1, floor(iter - warmup) / 1000)");

      print_help_option(&std::cout,
                        "refresh","int",
                        "Period between samples updating progress report print (0 for no printing)",
                        "default = max(1,iter/200))");

      print_help_option(&std::cout,
                        "leapfrog_steps","int",
                        "Number of leapfrog steps; -1 for no-U-turn adaptation",
                        "default = -1");

      print_help_option(&std::cout,
                        "max_treedepth","int",
                        "Limit NUTS leapfrog steps to 2^max_tree_depth; -1 for no limit",
                        "default = 10");
      
      print_help_option(&std::cout,
                        "epsilon","float",
                        "Initial value for step size, or -1 to set automatically",
                        "default = -1");
      
      print_help_option(&std::cout,
                        "epsilon_pm","[0,1]",
                        "Sample epsilon +/- epsilon * epsilon_pm",
                        "default = 0.0");

      print_help_option(&std::cout,
                        "equal_step_sizes","",
                        "Use same step size for every parameter with NUTS",
                        "default is to estimate varying step sizes during warmup");
      
      print_help_option(&std::cout,
                        "delta","[0,1]",
                        "Accuracy target for step-size adaptation (higher means smaller step sizes)",
                        "default = 0.5");

      print_help_option(&std::cout,
                        "gamma","+float",
			"Gamma parameter for dual averaging step-size adaptation",
			"default = 0.05");

      print_help_option(&std::cout,
                        "save_warmup","",
                        "Save the warmup samples");

      print_help_option(&std::cout,
                        "test_grad","",
                        "Test gradient calculations using finite differences");

      print_help_option(&std::cout,
                        "point_estimate","",
                        "Fit point estimate of hidden parameters by maximizing log joint probability");

      print_help_option(&std::cout,
                        "debug","",
                        "Save debug output into samples file");
      
      print_help_option(&std::cout,
			"nondiag_mass","",
			"Use a nondiagonal matrix to do the sampling");
        
      print_help_option(&std::cout,
			"cov_matrix","file",
			"Preset an estimated covariance matrix");

      print_help_option(&std::cout,
			"contour","(idx0,min,max,idx1,min,max,n=101)",
			"Generate contour information for graphical diagnostics."
			"Format is index of parameter 0 and its min/max, index of parameter 1 and its min/max, and the number of points");
      

      std::cout << std::endl;
    }

    bool do_print(int n, int refresh) {
      return (refresh > 0)
        && (n == 0
            || ((n + 1) % refresh == 0) );
    }

    template <class Sampler, class Model>
    void sample_from(Sampler& sampler,
                     bool epsilon_adapt,
                     int refresh,
                     int num_iterations,
                     int num_warmup,
                     int num_thin,
                     bool save_warmup,
		     bool debug,
                     std::ostream& sample_file_stream,
                     std::vector<double>& params_r,
                     std::vector<int>& params_i,
                     Model& model) {

      sampler.set_params(params_r,params_i);
     
      int it_print_width = std::ceil(std::log10(num_iterations));
      std::cout << std::endl;

      if (epsilon_adapt)
        sampler.adapt_on(); 
      for (int m = 0; m < num_iterations; ++m) {
        if (do_print(m,refresh)) {
          std::cout << "Iteration: ";
          std::cout << std::setw(it_print_width) << (m + 1)
                    << " / " << num_iterations;
          std::cout << " [" << std::setw(3) 
                    << static_cast<int>((100.0 * (m + 1))/num_iterations)
                    << "%] ";
          std::cout << ((m < num_warmup) ? " (Adapting)" : " (Sampling)");
          std::cout << std::endl;
          std::cout.flush();
        }
        if (m < num_warmup) {
          if (save_warmup && (m % num_thin) == 0) {
            stan::mcmc::sample sample = sampler.next();

            // FIXME: use csv_writer arg to make comma optional?
            sample_file_stream << sample.log_prob() << ',';
            sampler.write_sampler_params(sample_file_stream);
            sample.params_r(params_r);
            sample.params_i(params_i);
            model.write_csv(params_r,params_i,debug,sample_file_stream);
          } else {
            sampler.next(); // discard
          }
        } else {
          if (epsilon_adapt && sampler.adapting()) {
            sampler.adapt_off();
            sampler.write_adaptation_params(sample_file_stream);
          }
          if (((m - num_warmup) % num_thin) != 0) {
            sampler.next();
            continue;
          } else {
            stan::mcmc::sample sample = sampler.next();

            // FIXME: use csv_writer arg to make comma optional?
            sample_file_stream << sample.log_prob() << ',';
            sampler.write_sampler_params(sample_file_stream);
            sample.params_r(params_r);
            sample.params_i(params_i);
            model.write_csv(params_r,params_i,debug,sample_file_stream);
          }
        }
      }
    }

    void write_comment(std::ostream& o) {
      o << "#" << std::endl;
    }
    template <typename M>
    void write_comment(std::ostream& o,
                       const M& msg) {
      o << "# " << msg << std::endl;
    }
    template <typename K, typename V>
    void write_comment_property(std::ostream& o,
                                const K& key,
                                const V& val) {
      o << "# " << key << "=" << val << std::endl;
    }

    void write_point_estimate_comment_block(std::ostream& sample_stream,
					    const std::string& data_file,
					    const std::string& init_val,
					    const bool save_warmup,
					    const unsigned int random_seed) {
      write_comment(sample_stream,"Point Estimate Generated by Stan");
      write_comment(sample_stream);
      write_comment_property(sample_stream,"stan_version_major",stan::MAJOR_VERSION);
      write_comment_property(sample_stream,"stan_version_minor",stan::MINOR_VERSION);
      write_comment_property(sample_stream,"stan_version_patch",stan::PATCH_VERSION);
      write_comment_property(sample_stream,"data",data_file);
      write_comment_property(sample_stream,"init",init_val);
      write_comment_property(sample_stream,"save_warmup",save_warmup);
      write_comment_property(sample_stream,"seed",random_seed);
      write_comment(sample_stream);
    }

    void write_sample_comment_block(std::ostream& o,
				    const std::string& data_file,
				    const std::string& init_val,
				    const bool append_samples,
				    const bool save_warmup,
				    const unsigned int random_seed,
				    const int chain_id,
				    const int num_iterations,
				    const int num_warmup,
				    const unsigned int num_thin,
				    const bool equal_step_sizes,
				    const int leapfrog_steps,
				    const int max_treedepth,
				    const double epsilon,
				    const double epsilon_pm,
				    const double delta,
				    const double gamma,
				    const bool debug) {
      write_comment(o,"Samples Generated by Stan");
      write_comment(o);
      write_comment_property(o,"stan_version_major",stan::MAJOR_VERSION);
      write_comment_property(o,"stan_version_minor",stan::MINOR_VERSION);
      write_comment_property(o,"stan_version_patch",stan::PATCH_VERSION);
      write_comment_property(o,"data",data_file);
      write_comment_property(o,"init",init_val);
      write_comment_property(o,"append_samples",append_samples);
      write_comment_property(o,"save_warmup",save_warmup);
      write_comment_property(o,"seed",random_seed);
      write_comment_property(o,"chain_id",chain_id);
      write_comment_property(o,"iter",num_iterations);
      write_comment_property(o,"warmup",num_warmup);
      write_comment_property(o,"thin",num_thin);
      write_comment_property(o,"equal_step_sizes",equal_step_sizes);
      write_comment_property(o,"leapfrog_steps",leapfrog_steps);
      write_comment_property(o,"max_treedepth",max_treedepth);
      write_comment_property(o,"epsilon",epsilon);
      write_comment_property(o,"epsilon_pm",epsilon_pm);
      write_comment_property(o,"delta",delta);
      write_comment_property(o,"gamma",gamma);
      write_comment_property(o,"debug",debug);
      write_comment(o);
    }

    template <class Model>
    int nuts_command(int argc, const char* argv[]) {

      stan::io::cmd_line command(argc,argv);

      if (command.has_flag("help")) {
        print_nuts_help(argv[0]);
        return 0;
      }

      std::string data_file;
      command.val("data",data_file);
      std::fstream data_stream(data_file.c_str(),
                               std::fstream::in);
      stan::io::dump data_var_context(data_stream);
      data_stream.close();

      Model model(data_var_context, &std::cout);

      bool point_estimate = command.has_flag("point_estimate");

      std::string sample_file = "samples.csv";
      command.val("samples",sample_file);

      bool debug = command.has_flag("debug");
      
      unsigned int num_iterations = 2000U;
      command.val("iter",num_iterations);
      
      unsigned int num_warmup = num_iterations / 2;
      command.val("warmup",num_warmup);
      
      unsigned int calculated_thin = (num_iterations - num_warmup) / 1000U;
      unsigned int num_thin = (calculated_thin > 1) ? calculated_thin : 1U;
      command.val("thin",num_thin);

      bool user_supplied_thin = command.has_key("thin");

      int leapfrog_steps = -1;
      command.val("leapfrog_steps",leapfrog_steps);

      double epsilon = -1.0;
      command.val("epsilon",epsilon);

      int max_treedepth = 10;
      command.val("max_treedepth",max_treedepth);

      double epsilon_pm = 0.0;
      command.val("epsilon_pm",epsilon_pm);

      bool epsilon_adapt = epsilon <= 0.0;

      bool equal_step_sizes = command.has_flag("equal_step_sizes");

      double delta = 0.5;
      command.val("delta", delta);

      double gamma = 0.05;
      command.val("gamma", gamma);

      int refresh = num_iterations / 200;
      refresh = refresh <= 0 ? 1 : refresh; // just for default
      command.val("refresh",refresh);
        
      bool nondiag_mass = command.has_flag("nondiag_mass");
    
      std::string cov_file = "";
      command.val("cov_matrix", cov_file);

      contour_info contour;
      if (command.has_key("contour")) {
	std::string contour_string;
	command.val("contour", contour_string);
	if (contour.populate(contour_string) == false) {
	  std::cerr << "contour value is not formatted properly; " 
		    << "expecting (idx0,min,max,idx1,min,max,n)"
		    << std::endl;
	  return -1;
	}
      }
      

      unsigned int random_seed = 0;
      if (command.has_key("seed")) {
        bool well_formed = command.val("seed",random_seed);
        if (!well_formed) {
          std::string seed_val;
          command.val("seed",seed_val);
          std::cerr << "value for seed must be integer"
                    << "; found value=" << seed_val << std::endl;
          return -1;
        }
      } else {
        random_seed 
          = (boost::posix_time::microsec_clock::universal_time() -
             boost::posix_time::ptime(boost::posix_time::min_date_time))
          .total_milliseconds();
      }

      int chain_id = 1;
      if (command.has_key("chain_id")) {
        bool well_formed = command.val("chain_id",chain_id);
        if (!well_formed || chain_id < 0) {
          std::string chain_id_val;
          command.val("chain_id",chain_id_val);
          std::cerr << "value for chain_id must be positive integer"
                    << "; found chain_id=" << chain_id_val
                    << std::endl;
          return -1;
        }
      }
      
      // FASTER, but no parallel guarantees:
      // typedef boost::mt19937 rng_t;
      // rng_t base_rng(static_cast<unsigned int>(random_seed + chain_id - 1);

      typedef boost::ecuyer1988 rng_t;
      rng_t base_rng(random_seed);
      // (2**50 = 1T samples, 1000 chains)
      static boost::uintmax_t DISCARD_STRIDE = static_cast<boost::uintmax_t>(1) << 50;
      // DISCARD_STRIDE <<= 50;
      base_rng.discard(DISCARD_STRIDE * (chain_id - 1));
      
      std::vector<int> params_i;
      std::vector<double> params_r;

      std::string init_val;
      // parameter initialization
      int num_init_tries = 1;  // up here for printing below
      if (command.has_key("init")) {
        num_init_tries = -1;
        command.val("init",init_val);
        if (init_val == "0") {
          params_i = std::vector<int>(model.num_params_i(),0);
          params_r = std::vector<double>(model.num_params_r(),0.0);
        } else {
          try {
            std::fstream init_stream(init_val.c_str(),std::fstream::in);
            if (init_stream.fail()) {
              std::string msg("ERROR: specified init file does not exist: ");
              msg += init_val;
              throw std::invalid_argument(msg);
            }
            stan::io::dump init_var_context(init_stream);
            init_stream.close();
            model.transform_inits(init_var_context,params_i,params_r);
          } catch (const std::exception& e) {
            std::cerr << "Error during user-specified initialization:" 
                      << std::endl
                      << e.what() 
                      << std::endl;
            return -5;
          }
        }
      } else {
        init_val = "random initialization";  // for I/O
        // init_rng generates uniformly from -2 to 2
        boost::random::uniform_real_distribution<double> 
          init_range_distribution(-2.0,2.0);
        boost::variate_generator<rng_t&, 
	  boost::random::uniform_real_distribution<double> >
          init_rng(base_rng,init_range_distribution);

        params_i = std::vector<int>(model.num_params_i(),0);
        params_r = std::vector<double>(model.num_params_r());

        // retry inits until get a finite log prob value
        std::vector<double> init_grad;
        static int MAX_INIT_TRIES = 100;
        for (num_init_tries = 1; num_init_tries <= MAX_INIT_TRIES; ++num_init_tries) {
          for (size_t i = 0; i < params_r.size(); ++i)
            params_r[i] = init_rng();
          // FIXME: allow config vs. std::cout
          double init_log_prob = model.grad_log_prob(params_r,params_i,init_grad,&std::cout);
          if (!boost::math::isfinite(init_log_prob))
            continue;
          for (size_t i = 0; i < init_grad.size(); ++i)
            if (!boost::math::isfinite(init_grad[i]))
              continue;
          break;
        }
        if (num_init_tries > MAX_INIT_TRIES) {
          std::cout << "Initialization failed after " << MAX_INIT_TRIES 
                    << " attempts. "
                    << " Try specifying initial values,"
                    << " reducing ranges of constrained values,"
                    << " or reparameterizing the model."
                    << std::endl;
          return -1;
        }
      }

      bool save_warmup = command.has_flag("save_warmup");

      bool append_samples = command.has_flag("append_samples");
      std::ios_base::openmode samples_append_mode
        = append_samples
        ? (std::fstream::out | std::fstream::app)
        : std::fstream::out;

      if (command.has_flag("test_grad")) {
        std::cout << std::endl << "TEST GRADIENT MODE" << std::endl;
        return model.test_gradients(params_r,params_i);
      }

      if (point_estimate) {
        std::cout << "STAN OPTIMIZATION COMMAND" << std::endl;
        if (data_file == "")
          std::cout << "data = (specified model requires no data)" << std::endl;
        else 
          std::cout << "data = " << data_file << std::endl;

        std::cout << "init = " << init_val << std::endl;
        if (num_init_tries > 0)
          std::cout << "init tries = " << num_init_tries << std::endl;

        std::cout << "output = " << sample_file << std::endl;
        std::cout << "save_warmup = " << save_warmup<< std::endl;

        std::cout << "seed = " << random_seed 
                  << " (" << (command.has_key("seed") 
                              ? "user specified"
                              : "randomly generated") << ")"
                  << std::endl;

        std::fstream sample_stream(sample_file.c_str(), 
                                   samples_append_mode);
      
	write_point_estimate_comment_block(sample_stream,
					   data_file,
					   init_val,
					   save_warmup,
					   random_seed);

        sample_stream << "lp__,"; // log probability first
        model.write_csv_header(sample_stream,debug);

        std::vector<double> gradient;
        double lp = model.grad_log_prob(params_r, params_i, gradient);
        
        double lastlp = lp - 1;
        std::cout << "initial log joint probability = " << lp << std::endl;
        int m = 0;
        while ((lp - lastlp) / fabs(lp) > 1e-8) {
          lastlp = lp;
          lp = stan::optimization::newton_step(model, params_r, params_i);
          std::cout << "Iteration ";
          std::cout << std::setw(2) << (m + 1) << ". ";
          std::cout << "Log joint probability = " << std::setw(10) << lp;
          std::cout << ". Improved by " << (lp - lastlp) << ".";
          std::cout << std::endl;
          std::cout.flush();
          m++;
	  //           for (size_t i = 0; i < params_r.size(); i++)
	  //             fprintf(stderr, "%f ", params_r[i]);
	  //           fprintf(stderr, "   %f  (last = %f)\n", lp, lastlp);
          if (save_warmup) {
            sample_stream << lp << ',';
            model.write_csv(params_r,params_i,debug,sample_stream);
          }
        }

        sample_stream << lp << ',';
        model.write_csv(params_r,params_i,debug,sample_stream);

        return 0;
      }      

      std::cout << "STAN SAMPLING COMMAND" << std::endl;
      if (data_file == "")
        std::cout << "data = (specified model requires no data)" << std::endl;
      else 
        std::cout << "data = " << data_file << std::endl;

      std::cout << "init = " << init_val << std::endl;
      if (num_init_tries > 0)
        std::cout << "init tries = " << num_init_tries << std::endl;

      std::cout << "samples = " << sample_file << std::endl;
      std::cout << "append_samples = " << append_samples << std::endl;
      std::cout << "save_warmup = " << save_warmup<< std::endl;
      
      std::cout << "seed = " << random_seed 
                << " (" << (command.has_key("seed") 
                            ? "user specified"
                            : "randomly generated") << ")"
                << std::endl;
      std::cout << "chain_id = " << chain_id
                << " (" << (command.has_key("chain_id")
                            ? "user specified"
                            : "default") << ")"
                << std::endl;
      
      std::cout << "iter = " << num_iterations << std::endl;
      std::cout << "warmup = " << num_warmup << std::endl;
      std::cout << "thin = " << num_thin
                << (user_supplied_thin ? " (user supplied)" : " (default)")
                << std::endl;
      
      std::cout << "equal_step_sizes = " << equal_step_sizes << std::endl;
      std::cout << "leapfrog_steps = " << leapfrog_steps << std::endl;
      std::cout << "max_treedepth = " << max_treedepth << std::endl;;
      std::cout << "epsilon = " << epsilon << std::endl;;
      std::cout << "epsilon_pm = " << epsilon_pm << std::endl;;
      std::cout << "delta = " << delta << std::endl;
      std::cout << "gamma = " << gamma << std::endl;
      
      std::fstream sample_stream(sample_file.c_str(), 
				 samples_append_mode);
      

      write_sample_comment_block(sample_stream,
				 data_file, init_val, append_samples, 
				 save_warmup, random_seed, chain_id,
				 num_iterations, num_warmup, num_thin,
				 equal_step_sizes, leapfrog_steps, max_treedepth,
				 epsilon, epsilon_pm, delta, gamma, debug);

        
      clock_t start = clock();
      if (cov_file != ""){
	stan::mcmc::nuts_massgiven<rng_t> nuts_massgiven_sampler(model,
								 cov_file,
								 max_treedepth, epsilon,
								 epsilon_pm, epsilon_adapt,
								 delta, gamma,
								 base_rng, &params_r,
								 &params_i);
            
	// cut & paste (see below) to enable sample-specific params
	if (!append_samples) {
	  sample_stream << "lp__,"; // log probability first
	  nuts_massgiven_sampler.write_sampler_param_names(sample_stream);
	  model.write_csv_header(sample_stream,debug);
	}
	nuts_massgiven_sampler.set_error_stream(std::cout);  // cout intended
	nuts_massgiven_sampler.set_output_stream(std::cout);
            
	sample_from(nuts_massgiven_sampler,epsilon_adapt,refresh,
		    num_iterations,num_warmup,num_thin,save_warmup,debug,
		    sample_stream,params_r,params_i,
		    model);//Yuanjun add a comment here to allow nondiag_mass matrix
            
      }
      else if (nondiag_mass){
	stan::mcmc::nuts_nondiag<rng_t> nuts_nondiag_sampler(model,
							     max_treedepth, epsilon,
							     epsilon_pm, epsilon_adapt,
							     delta, gamma,
							     base_rng, &params_r,
							     &params_i);
            
	// cut & paste (see below) to enable sample-specific params
	if (!append_samples) {
	  sample_stream << "lp__,"; // log probability first
	  nuts_nondiag_sampler.write_sampler_param_names(sample_stream);
	  model.write_csv_header(sample_stream,debug);
	}
	nuts_nondiag_sampler.set_error_stream(std::cout);  // cout intended
	nuts_nondiag_sampler.set_output_stream(std::cout);
            
	sample_from(nuts_nondiag_sampler,epsilon_adapt,refresh,
		    num_iterations,num_warmup,num_thin,save_warmup,debug,
		    sample_stream,params_r,params_i,
		    model);//Yuanjun add a comment here to allow nondiag_mass matrix
      }
      else if (leapfrog_steps < 0 && !equal_step_sizes) {
        // NUTS II (with varying step size estimation during warmup)
        stan::mcmc::nuts_diag<rng_t> nuts2_sampler(model, 
                                                   max_treedepth, epsilon, 
                                                   epsilon_pm, epsilon_adapt,
                                                   delta, gamma, 
                                                   base_rng, &params_r,
                                                   &params_i);

        // cut & paste (see below) to enable sample-specific params
        if (!append_samples) {
          sample_stream << "lp__,"; // log probability first
          nuts2_sampler.write_sampler_param_names(sample_stream);
          model.write_csv_header(sample_stream,debug);
        }
        nuts2_sampler.set_error_stream(std::cout);  // cout intended
        nuts2_sampler.set_output_stream(std::cout); 

        sample_from(nuts2_sampler,epsilon_adapt,refresh,
                    num_iterations,num_warmup,num_thin,save_warmup,debug,
                    sample_stream,
		    params_r,params_i,model);

      } else if (leapfrog_steps < 0 && equal_step_sizes) {

        // NUTS I (equal step sizes)
        stan::mcmc::nuts<rng_t> nuts_sampler(model, 
                                             max_treedepth, epsilon, 
                                             epsilon_pm, epsilon_adapt,
                                             delta, gamma, 
                                             base_rng, &params_r,
                                             &params_i);

        nuts_sampler.set_error_stream(std::cout);
        nuts_sampler.set_output_stream(std::cout); // cout intended
	// cut & paste (see below) to enable sample-specific params
        if (!append_samples) {
          sample_stream << "lp__,"; // log probability first
          nuts_sampler.write_sampler_param_names(sample_stream);
          model.write_csv_header(sample_stream,debug);
        }

        sample_from(nuts_sampler,epsilon_adapt,refresh,
                    num_iterations,num_warmup,num_thin,save_warmup,debug,
                    sample_stream,
		    params_r,params_i,model);

      } else {

        // STANDARD HMC
        stan::mcmc::adaptive_hmc<rng_t> hmc_sampler(model,
                                                    leapfrog_steps,
                                                    epsilon, epsilon_pm, epsilon_adapt,
                                                    delta, gamma,
                                                    base_rng, &params_r,
                                                    &params_i);

        hmc_sampler.set_error_stream(std::cout); // intended
        hmc_sampler.set_output_stream(std::cout);
        // cut & paste (see above) to enable sample-specific params
        if (!append_samples) {
          sample_stream << "lp__,"; // log probability first
          hmc_sampler.write_sampler_param_names(sample_stream);
          model.write_csv_header(sample_stream,debug);
        }

        sample_from(hmc_sampler,epsilon_adapt,refresh,
                    num_iterations,num_warmup,num_thin,save_warmup,debug,
                    sample_stream,
		    params_r,params_i,model);
      }
      clock_t end = clock();
      double deltaT = (double)(end - start) / CLOCKS_PER_SEC;
      std::cout<<"used " << deltaT << " seconds" <<std::endl;
      
      sample_stream.close();
      std::cout << std::endl << std::endl;
      return 0;
    }

  } // namespace prob


} // namespace stan

#endif
