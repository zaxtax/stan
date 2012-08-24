---
layout: post
title: "Getting Started with Rstan"
date: 2012-08-24 12:50
comments: true
categories: 
---

RStan is the [R] interface for [Stan]. This howto includes how to install RStan, and how to use RStan.
<!-- more -->

## Prerequisites

### R 

Obviously we need [R]. In particular, R version 2.15.1 is needed. This is because one package that RStan depends on
requires R 2.15.1. 

### Other R packages 

As an R package, rstan depends directly on packages [Rcpp], [RcppEigen], and [inline].  So before installing rstan, these packages need to be installed:

``` r Install Required Packages
install.packages(c("inline", "Rcpp", "RcppEigen"))
```

### C++ compiler 

The same as Stan, RStan requires a C++ compiler that can be accessed by R. See Stan's manual on how to install a C++ compiler on the platform of interest. In short, a C++ compiler for Mac/Windows/Linux can be obtained using the following information. 

- **Mac OS X** [Xcode] from Apple  
- **Windows** GCC in [Rtools]
- **Linux** GCC or Clang. Generally it is easy to use a package management tool in your Linux distribution such as _apt_ on Debian (or Debian based) and _yum_ on Fedora (or Fedora based). For example, use `apt-get install build-essential` on Debian.

We can use the following R code to see if there is a C++ compiler that can be used by R. 

``` r using inline to compile C++ code on the fly
library(inline) 
library(Rcpp)
src <- ' 
  std::vector<std::string> s; 
  s.push_back("hello");
  s.push_back("world");
  return Rcpp::wrap(s);
'
hellofun <- cxxfunction(body = src, includes = '', 
                        plugin = 'Rcpp', verbose = FALSE)
cat(hellofun(), '\n') 
```

If you run the above lines in R and you have C++, you'll get `hello world` to appear in your R console.

#### Modify C++ compiler optimization level 

This is optional. But changing the optimization level for compiling models can improve the speed of sampling. 
We could change it as follows:

Run the following R code in R, to obtain the full path of file `Makeconf`
``` r Find the Makeconf that R uses
file.path(R.home(component = 'etc'), .Platform$r_arch, 'Makeconf')
## [1] "/Library/Frameworks/R.framework/Resources/etc/x86_64/Makeconf" ## on OS X
## [1] "/usr/lib64/R/etc//Makeconf" ## on Ubuntu
```

Look for the line beginning with `CXXFLAGS` in file _Makeconf_, it might look like (it can be a little different depending on platforms) 
``` make Change this line in file you found above
CXXFLAGS = -g -O2 $(LTO)
``` 

Change the above `-O2` to `-O3` 
     

# How to Install RStan #

Currently, RStan is not on [CRAN], a different `repos` for 
`install.packages` can be used to install rstan.  That is, instead we call 
`install.packages` to install rstan _from source_. 

``` r
## add current repository of rstan
options(repos = c(getOption("repos"), rstan = "http://wiki.stan.googlecode.com/git/R"))
install.packages('rstan', type = 'source')
```

``` r Install using 'contriburl' instead of 'repos'
install.packages('rstan', contrib = 'http://wiki.stan.googlecode.com/git/R/bin/macosx/leopard/contrib/2.15/')
```

In addition, if the binary version of package rstan is not available, we can explicitly install the package from source as follows. 

``` r Set a package repository
options(repos = c(getOption("repos"), rstan = "http://wiki.stan.googlecode.com/git/R"))
install.packages('rstan', type = 'source')
```


# How to Use RStan

### Load rstan
The package names is _rstan_, so we need to use `library(rstan)` to load the package. 

``` r Load the rstan package
library(rstan) 
```

 
### Example 1: The Eight Schools 

This is an example in Section 5.5 of Gelman _et al_ (2003), which studied
coaching effects from eight schools. For simplicity, we call this exmaple
"eight schools." 


``` r Eight Schools
schools.code <- '
  data {
    int<lower=0> J; // number of schools 
    real y[J]; // estimated treatment effects
    real<lower=0> sigma[J]; // s.e. of effect estimates 
  } 
  parameters {
    real theta[J]; 
    real mu; 
    real<lower=0> tau; 
  } 
  model {
    theta ~ normal(mu, tau); 
    y ~ normal(theta, sigma);
  } 
'

schools.dat <- list(J = 8, 
                    y = c(28,  8, -3,  7, -1,  1, 18, 12),
                    sigma = c(15, 10, 16, 11,  9, 11, 10, 18))

fit <- stan(model.code = schools.code, data = schools.dat, 
            n.iter = 1000, n.chains = 4)

``` 

In this example, we can also store the model's Stan code 
in a file, or we can download [8schools.stan] (reproduced below) to our working
directory and use the following usage of function `stan` instead. 

``` stan 8schools.stan http://wiki.stan.googlecode.com/git/rstangettingstarted/8schools.stan
data {
    int<lower=0> J; // number of schools 
    real y[J]; // estimated treatment effects
    real<lower=0> sigma[J]; // s.e. of effect estimates 
  } 
parameters {
    real theta[J]; 
    real mu; 
    real<lower=0> tau; 
  } 
  model {
    theta ~ normal(mu, tau); 
    y ~ normal(theta, sigma);
  } 
```

``` r using an external file
fit1 <- stan(file = '8schools.stan', data = schools.dat, 
             n.iter = 1000, n.chains = 4)
```

Once a model is fitted, we can use the fitted result as an input to 
fit the model with other data or settings. This would save us time
of compiling the C++ code for the model. By specifying parameter
`fit` for function `stan`, we can fit the model again. For example,
if we want to sample more iterations, we can proceed as follows. 

``` r
fit2 <- stan(fit = fit1, data = schools.dat, n.iter = 10000, n.chains = 4)
```


In addition, as in BUGS (or JAGS), Stan (different from RStan here) needs all the data to be in an R dump file. In the case we have this file, rstan provides function `read.rdump` to read all the data into an R list.  For example, we have a file named _8schools.rdump_ that contains the following text in our working directory. 


``` r
J <- 8
y <- c(28,  8, -3,  7, -1,  1, 18, 12)
sigma_y <- c(15, 10, 16, 11,  9, 11, 10, 18)
```


Then we can read the data from "8schools.rdump" as follows. 

``` r 
schools.dat <- read.rdump('8schools.rdump')
```

The R dump file actually can be sourced using function `source` in R, which however by default would 
read all the data into the global environment. In this case, we can specify the data for function `stan` using
object names. That is, 

``` r
source('8schools.rdump') 
fit <- stan(file = '8schools.stan', data = c("J", "y", "sigma_y"), 
            n.iter = 1000, n.chains = 4) 
```
 

### Example 2: The Rats 

The Rats example is also a popular example. For example, we can find the [OpenBUGS] version [here](http://www.openbugs.info/Examples/Rats.html), which originally is from Gelfand _et al_ (1990). 
The data are about the growth of 30 rats weekly for five weeks. 
In the linked data [rats.txt], we list the data, in which we use _x_ to denote the dates
the data were collected. 

``` stan rats.stan  http://stan.googlecode.com/git/src/models/bugs_examples/vol1/rats/rats.stan
// http://www.mrc-bsu.cam.ac.uk/bugs/winbugs/Vol1.pdf
// Page 3: Rats
data {
  int<lower=0> N;
  int<lower=0> T;
  real x[T];
  real y[N,T];
  real xbar;
}
parameters {
  real alpha[N];
  real beta[N];

  real mu_alpha;
  real mu_beta;          // beta.c in original bugs model

  real<lower=0> sigmasq_y;
  real<lower=0> sigmasq_alpha;
  real<lower=0> sigmasq_beta;
}
transformed parameters {
  real<lower=0> sigma_y;       // sigma in original bugs model
  real<lower=0> sigma_alpha;
  real<lower=0> sigma_beta;

  sigma_y <- sqrt(sigmasq_y);
  sigma_alpha <- sqrt(sigmasq_alpha);
  sigma_beta <- sqrt(sigmasq_beta);
}
model {
  mu_alpha ~ normal(0, 100);
  mu_beta ~ normal(0, 100);
  sigmasq_y ~ inv_gamma(0.001, 0.001);
  sigmasq_alpha ~ inv_gamma(0.001, 0.001);
  sigmasq_beta ~ inv_gamma(0.001, 0.001);
  alpha ~ normal(mu_alpha, sigma_alpha); // vectorized
  beta ~ normal(mu_beta, sigma_beta);  // vectorized
  for (n in 1:N)
    for (t in 1:T) 
      y[n,t] ~ normal(alpha[n] + beta[n] * (x[t] - xbar), sigma_y);

}
generated quantities {
  real alpha0;
  alpha0 <- mu_alpha - xbar * mu_beta;
}
```

``` r R code to run the Rats model
library(rstan)  
y <- read.table('rats.txt', header = TRUE)
y <- data.matrix(y)
x <- c(8, 15, 22, 29, 36)
rats.dat <- list(N = nrow(y), T = ncol(y), 
                 x = x, y = y, xbar = mean(x))
rats.fit <- stan(file = 'rats.stan', data = rats.dat, verbose = FALSE)
``` 

# Need More Help? 

More details about RStan can be found in the documentation including the vignette of package rstan. 
For example, using `help(stan)` and `help("stanfit-class")` to check out the help for function `stan` 
and S4 class `stanfit`.  
And see Stan's manual for details about Stan and Stan modeling language.

## Reference 
 * Gelman, A., Carlin, J. B., Stern, H. S., and Rubin, D. B. (2003). _Bayesian Data Analysis_, CRC Press, London, 2nd Edition. 
 * The Stan Development Team (2012).  [Stan Modeling Language: User's Guide and Reference](http://stan.googlecode.com/files/stan-reference.pdf).  
 * Gelfand, A. E., Hills S. E., Racine-Poon, A., and Smith A. F. M. (1990). "Illustration of Bayesian Inference in Normal Data Models Using Gibbs Sampling", Journal of the American Statistical Association, 85, 972-985. 
 * [Stan] 
 * [R] 
 * [BUGS]
 * [OpenBUGS]   
 * [JAGS] 
 * [Rcpp]

 
 
[R]:         http://R-project.org
[Stan]:      http://mc-stan.org
[Rcpp]:      http://cran.r-project.org/web/packages/Rcpp/index.html
[RcppEigen]: http://cran.r-project.org/web/packages/RcppEigen/index.html
[inline]:    http://cran.r-project.org/web/packages/inline/index.html
[Xcode]:     https://developer.apple.com/xcode/
[Rtools]:    http://cran.r-project.org/bin/windows/Rtools/
[CRAN]:      http://cran.r-project.org/
[8schools.stan]: http://wiki.stan.googlecode.com/git/rstangettingstarted/8schools.stan
[OpenBUGS]:  http://www.openbugs.info
[rats.txt]:  http://wiki.stan.googlecode.com/git/rstangettingstarted/rats.txt
[rats.stan]: http://stan.googlecode.com/git/src/models/bugs_examples/vol1/rats/rats.stan
[BUGS]:      http://www.mrc-bsu.cam.ac.uk/bugs/
[JAGS]:      http://mcmc-jags.sourceforge.net/