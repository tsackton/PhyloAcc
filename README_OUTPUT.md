# Format of output files
*prefix*\_rate_postZ\_\*.txt: posterior median of conserved and accelerated rate and posterior mean of latent state on each branch for each element. Columns in the file are:
1. element No. which is order of the element in the input bed file starting from zero
2. posterior mode of accelerated mutation rate
3. posterior mode of conserved mutation rate
4. from 4th column and on, we have four columns for each branch:\*\_0 indicates whether it's "missing"; \*\_1, \*\_2 and \*\_3 are the posterior probability in neutral, conserved and accelerated state respectively. The algorithm will prune "missing" branches within outgroup and set the latent states of them to -1 so that the three posterior probabilities are all zero. Column names indicate the branch right above the node and the order of the branch is the same as that in *prefix*\_elem_Z.txt. 

If sampling hyperparameters, posterior medians/means under different hyperparameters will be concantenated to this file. If an element is filtered because of too many alignment gaps (criteria see [README2.md](PhyloAcc/README2.md)), it will not appear in this file.

*prefix*\_elem_lik.txt: marginal logliklihood for all models (integrating out parameters). The columns are:
  * *No.*: The order of the element in the input bed file starting from zero
  * *ID*: The element name as in the iput bed file
  * *loglik_NUll*: marginal logliklihood under null model
  * *loglik_RES*: marginal logliklihood under accelerated model
  * *loglik_all*: marginal logliklihood under full model
  * *log_ratio*: Bayes factor between null and accelerated model
  * *loglik_Max1, loglik_Max2, loglik_Max3*: Maximum joint likelihood of X (sequences), r (mutation rates) and Z (latent states) under null ($M_0$), full ($M_2$) and accelerated ($M_1$) model respectively.

If updating hyperparameters, the algorithm will only compute the log-likelihood under full model. When the hyperparameters are updated, the log-likelihoods for each element will be recomputed and concatenated to this file. If an element is filtered because of too many alignment gaps (criteria see [README2.md](PhyloAcc/README2.md)), all the columns will be zero. If the MCMC algorithm is trapped at some local modes or some other numerical errors occur for some elements, it will return NA.
  

*prefix*\_1_elem_Z.txt: maximum loglikhood configurations of latent state Z under full model, with Z=-1(if the element is 'missing' in the branches of outgroup species),0(neutral),1(conserved),2(accelerated); each row is an element, ordered same as the input bed file. Output this file if not sample hyperparameters. If an element is filtered because of too many alignment gaps (criteria see [README2.md](PhyloAcc/README2.md)), all the columns will be zero. 

*prefix*\_hyper.txt: hyperparameters at each iteration, only meaningful if adopting full Bayesian approach by sampling hyperparameters. Columns are:
* *iter*: the number of iteration
* *nprior_a, nprior_b*: the shape and scale hyperparameter of the gamma prior of accelerated mutation rate
* *cprior_a, cprior_b*: the shape and scale hyperparameter of the gamma prior of conserved mutation rate
* *indel, indel2*: empty columns, place holders
* *grate*: rate of gain conservation, i.e. the prior of transition probability of $Z=0$ to $Z=1$
* *lrate*: rate of loss conservation, i.e. the prior of transition probability of $Z=1$ to $Z=2$

*prefix*\_lik\_rate\_Z\_[0-2]_\*.txt: Output this file if verbose=T, which contains trace of MCMC samples in each iteration for an element. Each row is one iteration and columns are: log-likelihood, accelerated mutation rate, conserved mutation rate and latent state Z of each branch. If sampling hyperparameters, MCMC samples will be concantenated together under different hyperparameters. [0-2]: under null, full and accelerated model respectively. *: element No. .

The output files from the extended version including gBGC is slightly different. *prefix*\_rate_postZ\_*.txt* also contains posterior mode of gBGC coefficient (*gBC* column) and posterior of having gBGC effect on each branch (*\*_B* columns).*prefix*_1_elem_Z.txt has latent gBGC state on each branch in the first columns.
