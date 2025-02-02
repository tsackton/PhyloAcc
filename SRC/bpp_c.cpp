//
//  bpp_c.cpp
//  PhyloAcc_init2
//
//  Created by hzr on 4/19/16.
//  Copyright © 2016 hzr. All rights reserved.
//

#include "bpp_c.hpp"
#include "bpp.hpp"


//void BPP_C::getSubtree(int root, set<int>& child, vector<int> & visited_init)  // traverse from root, stop at children, 74 & 64; do include 1-S!
//{
//    
//    
//    int j = root;
//
//    if(child.find(j) != child.end())
//    {
//        
//        visited_init.push_back(j);
//        
//        return;
//    }
//    
//   
//        
//    for(int chi=0;chi<2;chi++)
//    {
//        if(children2[j][chi]!=-1)
//        getSubtree(children2[j][chi], child, visited_init);
//        
//    }
//        
//    
//    
//    visited_init.push_back(j);
//    
//}



void BPP_C::getSubtree(int root, vector<int> & visited_init)  // traverse from root to children, include root
{
    
    
    int j = root;
    
    for(int chi=0;chi<2;chi++)
    {
        if(children2[j][chi]!=-1)
            getSubtree(children2[j][chi], visited_init);
        
        
    }
    
       visited_init.push_back(j);
    
}



void BPP_C::getSubtree_missing(int ss, set<int> & upper, int child)  // traverse from root to children, include root
{
    
    if(upper.find(ss)==upper.end() || ss<S) return;
    
    int s = ss;
    while(upper.find(s)!=upper.end() && s>=S)
    {
        int* p = children2[s];
        //assert(p[0]!=-1 || p[1]!=-1);
        if(missing[p[0]] && !missing[p[1]])
        {
            parent2[p[0]] = -1;
            //root = s;
            distances2[p[1]] +=distances2[s];
            if(s == root)
            {
                root = p[1];
            }else{
                children2[parent2[ss]][child] = p[1];
                parent2[p[1]] = parent2[ss];
            }
            s = p[1];
        }
        else if(!missing[p[0]] && missing[p[1]])
        {
            parent2[p[1]] = -1;
            //root = s;
            distances2[p[0]] +=distances2[s];
            if(s == root)
            {
                root = p[0];
            }else{
                children2[parent2[ss]][child] = p[0];
                parent2[p[0]] = parent2[ss];
            }
            s = p[0];
        }
        else{
            
            getSubtree_missing(p[0], upper, 0);
            getSubtree_missing(p[1], upper, 1);
            return;
        }

    }
    
    

}


void BPP_C::getEmission_ambig() //whether calculate null
{
    //vec tmp(4); vec tmp2(4); vec tmp3(4);
    for(int s =0; s<S;s++) //N-1
    {
        if(missing[s]) continue;
        for(int g=0;g <GG;g++)
        {
            if(Tg[g][s] !=-1) continue;
            ambiguousS_null[g][s] = BPP::log_multi(log_cache_TM_null[s],lambda[g][s]);
        }
    }
}


void BPP_C::initMCMC(int iter, BPP&bpp, int resZ)  // assign small prob from Z = 1 to missing
{
    
    if(iter == 0)
    {
        //initalize Z, all Z are 1 except the root
        Z = vector<int>(N,-1);
       
        for(vector<int>::iterator it = nodes.begin();it<nodes.end();it++) Z[*it] = 1; //
        
        
        Z[root] = 0;
        //if(Z[bpp.moveroot]!=-1) Z[bpp.moveroot] =0;
    }else{
        
        Z = bpp.cur_Z[resZ][CC];
        
    }
    //Z = bpp.uniqZ2[0];  //initilize randomly, follow Markov Chain
    
    fixZ = vector<int>(N,-1);
    for(vector<int>:: iterator it = upper_c.begin(); it < upper_c.end(); it++)
    {
        fixZ[*it] = 1;
    }
    
    fixZ[root] =0;  //0: 0,1: 0/1, -1: all
    
    MaxLoglik = -INFINITY;
       

    
    trace_n_rate[0] = bpp.cur_nrate[CC]; //1;
    trace_c_rate[0] = bpp.cur_crate[CC]; //ratio; log_cache_TM_cons??
    
    for(int s=0; s<N; s++)
    {
        log_cache_TM_null[s] = bpp.log_cache_TM_null[s]; // already initialized in hpp
        log_TM_Int[s] = bpp.log_TM_Int[s];
    }


    for(vector<int>::iterator it = nodes.begin(); it < nodes.end() - 1; it++)
    {
        
        int s = *it;
        if(distances2[s]>0 )
        {
            if(distances2[s] != bpp.distances[s])
            {
                mat tmp_diag  = exp(bpp.eigenval*distances2[s]);
                mat x = bpp.eigenvec;
                x.each_col()%=tmp_diag;
                log_cache_TM_null[s] = log(bpp.eigeninv * x) ;
                
                //double z = exp(-bpp.ind_grate *distances2[s]);
                //log_TM_Int[s](0,0) = log(z);
                //log_TM_Int[s](1,0) = log(1-z);
                //double y = exp(-bpp.ind_lrate *distances2[s]);
                //log_TM_Int[s](1,1) = log(y);
                //log_TM_Int[s](2,1) = log(1-y);
                
            }
            
            mat tmp_diag  = exp(bpp.eigenval*distances2[s] * bpp.cur_nrate[CC]);
            mat x = bpp.eigenvec;
            x.each_col()%=tmp_diag;
            
            log_cache_TM_neut[s] = log(bpp.eigeninv * x) ; //transpose Q
            
            tmp_diag  = exp(bpp.eigenval*distances2[s] * bpp.cur_crate[CC]); //*scale_cons[c]);
            x = bpp.eigenvec;
            x.each_col()%=tmp_diag;
            
            log_cache_TM_cons[s] = log(bpp.eigeninv * x);
            
        }
        else{
            
            log_cache_TM_cons[s] = log_cache_TM_null[s] ;
            log_cache_TM_neut[s] = log_cache_TM_null[s] ;
        }
        
    }
    
    for(vector<int>:: iterator it = upper_c.begin(); it!=upper_c.end();it++)
    {
        log_TM_Int[*it](1,1) = 0;
        log_TM_Int[*it](2,1) = log(0);
    }

    
    log_prob_back = vector<vec> (N, zeros<vec>(3));
    
    for(int s=S; s<N; s++)
        for(int g=0; g<GG; g++)
            lambda[g][s].fill(0);
    
   getEmission_ambig();  
    //with prior, null model, all branches except outgroup are conserved, only have to sample Z  (0/1) of outgroup
    if(resZ==2)
    {
        //initiate log_prob_back not used
        
        for(int s=0; s<S;s++) //0/1
        {
            
            log_prob_back[s][1] = 0;
            log_prob_back[s][0] = 0;
            log_prob_back[s][2] = -INFINITY;
            fixZ[s] = 1;
        }
            
        for(vector<int>::iterator it = bpp.target_species.begin(); it < bpp.target_species.end(); it++) //1/2
        {
            int s= *it;
//            log_prob_back[s][2] = 0;
//            log_prob_back[s][1] = 0;
//            log_prob_back[s][0] = -INFINITY;
//            fixZ[s] = 2;
//            lambda2[s].col(0).fill(-INFINITY); lambda2[s].col(2).fill(0); // in last step set to -inf
            
            //restore restriction on 2
            log_prob_back[s][2] = 0;
            fixZ[s] = -1;

        }
        
              
    }
    else if(resZ==0)
    {
        for(vector<int>::iterator it = nodes.begin();it<nodes.end()-1;it++)  //don't include root, subtree doesn't have the root edge
        {
            int s = *it;
            
            log_prob_back[s][1] = 0;
            log_prob_back[s][0] = 0;
            log_prob_back[s][2] = -INFINITY;
            fixZ[s] = 1;
            
        }
        

   }
    
   //fixZ[bpp.moveroot] =0;

    //ctnutils::DispVector(fixZ);
    //cout << endl;
    
}

// ofstream & outZ, no use!
void BPP_C::Gibbs(int iter, BPP &bpp, ofstream & outZ, string output_path,string output_path2,int resZ, bool UpR, bool UpHyper, double lrate_prop, double grate_prop)
{
    
    vector<int> changedZ;
    vector<bool> visited = vector<bool>(N,false);
    double logp_Z = 0.0 ;
    //std::fill(visited.begin()+S,visited.end(),false);
    
    //initial lambda
    for(int g=0; g<GG; g++)
        Update_Tg(g, visited, bpp, false);  // get history for each base and get lambda;
    
    double log_lik_old = 0;
    
    for(m=0; m<(num_burn+num_mcmc); m++)
    {
        
        // monitoring and storaging trace loglik and Z
        if(m == 0 || changedZ.size()>0)
        {
            logp_Z = prior_Z_subtree(Z);
            for(vector<int>::iterator it = nodes.begin(); it < nodes.end(); it++)
            {
                if(missing[*it]) logp_Z += log_emission[*it][Z[*it]];// for missing
            }
            
        }
        
        
        MonitorChain(m, bpp, log_lik_old, logp_Z, resZ);  //cout in each function
        
        if(m == num_burn+num_mcmc-1) break;
        
        // sample H_m|Z_m,r_m
        for(int g=0; g<GG; g++)
            Update_Tg(g, visited, bpp, true);  // get history for each base and get lambda;

        
        // sample Z_(m+1)|H_m,r_m
        changedZ = Update_Z_subtree(bpp.num_base);
        
        
//            if(changedZ.size() == 0) //m >0 &&
//            {
//
//                // proposal
//                int propConf, revConf, changZp; //number of proposal from Z, and from propZ
//                //double gain, loss;
//                vector<int> propZ = Move_Z(propConf, revConf, changZp);
//                changedZ.push_back(changZp);
//                getUpdateNode(changedZ,visited);  // set lambda to zero!!
//
//                double prop_loglik = Update_f_Xz(log_pi, bpp.num_base, propZ,log_cache_TM_neut, log_cache_TM_cons, visited); // lambda changed!
//                //            log_f_Z(propZ, log_TM_Int, gain, loss);
//                //            prop_loglik += gain + loss;
//
//                //            log_f_Z(Z, log_TM_Int, gain, loss);
//                //            log_lik_old += gain + loss;
//
//                double MH = prop_loglik - log_lik_old - logp_Z + log(propConf) - log(revConf);
//                //cout << "prop_loglik: "<< prop_loglik << " orginal: "<< log_lik_old + logp_Z << " logp_Z: " << logp_Z << endl;
//
//                if(log(gsl_rng_uniform(RNG)) < MH)
//                {
//                    cout <<m << " changed Z: " << changZp << " " << Z[changZp] << " to " <<propZ[changZp] << endl;
//                    Z[changZp] = propZ[changZp];
//                    
//                }
//                
//            }
        
        getUpdateNode(changedZ,visited);  // set lambda to zero!!
        
        // sample r_(m+1)|Z_(m+1), r_m
        if(changedZ.size()>0)
        {
            log_lik_old = 0;
            // calculate marginal likelihood,
            for(int g=0;g<GG;g++)
            {
                Update_Tg(g, visited, bpp, false);
                log_lik_old += BPP::log_exp_sum(lambda[g][root]);
            }
        }
        


        if( UpR )// && m%num_thin==0)  // update transition rate
        {
            
            vector<bool> visited_neut = vector<bool>(N,true);
            vector<bool> visited_cons = vector<bool>(N,true);
    
            getUpdateNode(1,visited_neut);  //all neut nodes, no lambda
            getUpdateNode(0,visited_cons);  //all conserved nodes
            
            
            // sample neutral and conserved rate by MH, c has to be after n!
            trace_n_rate[m+1] = sample_rate(resZ, trace_n_rate[m],true, visited_neut, log_lik_old,bpp);  //partial lambda changed, given Z, integrate history
            trace_c_rate[m+1] = sample_rate(resZ, trace_c_rate[m],false, visited_cons, log_lik_old,bpp);
            
    
        }else{
            trace_n_rate[m+1] = trace_n_rate[m];
            trace_c_rate[m+1] = trace_c_rate[m];
        }
        
       
        

        
    }
    
    bpp.log_liks_Z[resZ][CC] = MaxLoglik;
    bpp.Max_Z[resZ][CC] = Max_Z;
    
    if(UpHyper) {
        bpp.log_liks_curZ[CC] = trace_loglik[m - 1]; bpp.cur_Z[resZ][CC] = Z;
        // compute Z transition matrix for proposal
        double loss, gain;
        log_f_Z(Z, log_TM_Int, gain, loss);  // only do this for full model, no constraint
        bpp.MH_ratio_gain[CC] = -gain;
        bpp.MH_ratio_loss[CC] = -loss;
        
        
        for(vector<int>::iterator it = nodes.begin(); it <nodes.end(); it++)
        {
            int s = *it;
            double z = 1 - grate_prop;
            log_TM_Int[s](0,0) = log(z);
            log_TM_Int[s](1,0) = log(1 - z);
            //double y = exp(-lrate_prop *distances2[s]);
            double y = 1 - lrate_prop;
            log_TM_Int[s](1,1) = log(y);
            log_TM_Int[s](2,1) = log(1-y);
            //log_TM_Int[s] = log(TM_Int[s]);
        }
        
        
        
        for(vector<int>:: iterator it = upper_c.begin(); it!=upper_c.end();it++)
        {
            log_TM_Int[*it](1,1) = 0;
            log_TM_Int[*it](2,1) = log(0);
            //log_TM_Int[*it] = log(TM_Int[*it]);
        }
        
        
        log_f_Z(Z, log_TM_Int, gain, loss);
        bpp.MH_ratio_gain[CC] += gain;
        bpp.MH_ratio_loss[CC] += loss;
    
        bpp.cur_crate[CC] = trace_c_rate[m-1];
        bpp.cur_nrate[CC] = trace_n_rate[m-1];  //doesn't matter, initalize Z with no 2, random sample r_n from prior
    }
    
    
    
    
   // if(verbose) //BPP::printZ(N, Max_Z,bpp.children);
     #pragma omp critical
    {
        if(verbose) {
            cout <<CC <<": " << Max_m << ", " << MaxLoglik << ", "<< trace_c_rate[Max_m] << ", " << trace_n_rate[Max_m] <<endl;
            for(int s=0;s<N;s++ )
            {
                if(Max_Z[s]!=1 && !missing[s])
                {
                    cout << s << ": " <<Max_Z[s] << ", ";
                }
            }
            cout << endl << "gap: ";
        
            for(int s=0;s<N;s++ )
            {
                if(missing[s])
                {
                    cout << s << ": " <<Max_Z[s] << ", ";
                }
            }
            cout << endl << endl;
        }
    }
    
    
    if(verbose) Output_sampling(iter, output_path2, bpp, resZ);  //has to be before Output_init!!

}













// only update probability of nodes above changedZ
void BPP_C::getUpdateNode(vector<int> changedZ, vector<bool> & visited_init) //changedZ from small to large (bottom to top)
{
    for(int i =0;i<N;i++)
        visited_init[i] = true;
    
    if(changedZ.size()==0) return;
    for(vector<int>::iterator it = changedZ.end()-1 ; it >= changedZ.begin(); --it){
        int j = *it;
        j = parent2[j];
        
        while(j!=N)
        {
            if(!visited_init[j]) break;
            visited_init[j] = 0;
            for(int g=0; g<GG; g++) lambda[g][j].zeros();  // only fathers of changedZ!
            
            j = parent2[j];
            assert(j!=-1);
            
        }
    }
    
}

void BPP_C::Update_Tg(int g, vector<bool> visited, BPP& bpp, bool tosample)  // impute base pair of each internal node (except missing nodes)
{
    
    if(!tosample)
    {
        // 1. sending the lambda msg from leaves bottom up through the network
        for(vector<int>::iterator it = internal_nodes.begin(); it <internal_nodes.end(); it++)  // didn't use leaves
        {
            int s = *it;
            if(visited[s] || missing[s] || Tg[g][s] == bpp.num_base)  continue; //
            int* p = children2[s];
            
            
            for(int cc=0;cc<2;cc++)
            {
                int chi = p[cc];
                if(missing[chi] || Tg[g][chi] == bpp.num_base ) continue; //
                if(Z[chi] ==2) lambda[g][s] +=  BPP::log_multi(log_cache_TM_neut[chi],lambda[g][chi]);
                else if (Z[chi] ==0) lambda[g][s] +=  BPP::log_multi(log_cache_TM_null[chi],lambda[g][chi]);
                else lambda[g][s] +=  BPP::log_multi(log_cache_TM_cons[chi],lambda[g][chi]);
            }
            
        }
        
        // 2. processing the distribution of root species
        
        if(!visited[root]) lambda[g][root] += log_pi;
    }
    else
    {
        vec prob = BPP::log_sample(lambda[g][root]);
        //cout<<prob.t() <<endl;
        
        //3. sequentially sample the tree top-down
        unsigned int n[bpp.num_base];
        gsl_ran_multinomial(RNG, bpp.num_base, 1,prob.memptr(),n);
        int nn;
        for(nn=0; nn<bpp.num_base;nn++)
        {
            if(n[nn]>0) break;
        }
        
        if(nn>bpp.num_base - 1 || nn<0) {
            
            for(int n=0;n<N;n++)
            {
                //cout<<lambda[c][g][n].t()<<endl;
            }
            cout<<g<<endl;
            
            throw runtime_error("Sample Tg root error");
            
        }
        
        Tg[g][root] = nn ;
        
        
        vec log_trans_p = zeros<vec>(bpp.num_base);
        vec trans_p(bpp.num_base);
        for(vector<int>::iterator it =internal_nodes.end()-2; it>=internal_nodes.begin(); it--)
        {
            int s = *it;
            int p = parent2[s];
            if(missing[s] || Tg[g][s] == bpp.num_base) continue; //
            
            if(Z[s] ==2) log_trans_p = log_cache_TM_neut[s].col(Tg[g][p]);  //trans_p *
            else if(Z[s] ==0) log_trans_p = log_cache_TM_null[s].col(Tg[g][p]);
            else log_trans_p = log_cache_TM_cons[s].col(Tg[g][p]);
            
            log_trans_p = log_trans_p + lambda[g][s];
            trans_p = BPP::log_sample(log_trans_p);
            
            
            
            unsigned int n[bpp.num_base];
            gsl_ran_multinomial(RNG, bpp.num_base, 1,trans_p.memptr(),n);
            
            
            int nn;
            for(nn=0; nn<bpp.num_base;nn++)
            {
                if(n[nn]>0) break;
            }
            if(nn>bpp.num_base - 1 || nn<0)
            {
                throw runtime_error("Sample Tg error");
            }
            Tg[g][s] = nn;
        }
    }
    
    
//    ctnutils::DispVector(Tg[g]);
//    cout<<endl;
    
}

void BPP_C::MonitorChain(int m, BPP &bpp, double & loglik, const double add_loglik,const int resZ)  //calculate P(X|Z, TM(r) ),
{
    if(m==0)
    {
        trace_loglik[m] = 0;
        // calculate marginal likelihood,
        for(int g=0;g<GG;g++)
        {
            trace_loglik[m] += BPP::log_exp_sum(lambda[g][root]); // for X
            
        }
        loglik =trace_loglik[m];
    }else{
        trace_loglik[m] = loglik;
    }
    
    
    // add prior of Z and prior of r
    //double gain, loss;
    //log_f_Z(Z, log_TM_Int, gain, loss);
    trace_full_loglik[m] = trace_loglik[m] + add_loglik ;
    trace_full_loglik[m] += log(gsl_ran_gamma_pdf(trace_c_rate[m],bpp.cprior_a,bpp.cprior_b));
    if(resZ !=0)
    {
        trace_full_loglik[m] += log(gsl_ran_gamma_pdf(trace_n_rate[m],bpp.nprior_a,bpp.nprior_b));
    }
    
    
    
    for(int s=0; s<N;s++)
    {
        trace_Z[m][s] = Z[s];
        
    }
    
    if(m>=num_burn && trace_full_loglik[m]>MaxLoglik)
    {
        MaxLoglik = trace_full_loglik[m];
        Max_Z = Z;
        Max_m = m;
    }
    
    if(m%500==0 && verbose){ 
        cout <<CC <<": " << trace_loglik[m] <<", " << trace_full_loglik[m]<< ", " << trace_n_rate[m] << ", " << trace_c_rate[m] <<", " <<  prop_c <<", " << prop_n << endl;
        //cout << "changed Z: ";
        //ctnutils::DispVector(changedZ);
        //cout << "number of Z changed: " << changedZ.size();
        for(int s=0;s<N;s++ )
        {
            if(Z[s]!=1)
            {
                cout << s << ": " <<Z[s] << ", ";
            }
        }
        
        cout << endl;
    }

    
}

void BPP_C::getUpdateNode(bool neut,vector<bool> & visited_init) //changedZ from small to large (bottom to top)
{
    
    //std::fill(visited_init, visited_init + N, 1);
    
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end();it++){
        int s = *it;
        if(missing[s])  continue;
        if(!neut && (Z[s]==0 ||  Z[s]==2)) continue;  // only get branches with Z == 1
        if(neut && (Z[s]==1 ||  Z[s]==0)) continue;   // only get branches with Z == 2
        int j = s;
        j = parent2[j];
        
        while(j!=N)
        {
            if(!visited_init[j]) break;
            visited_init[j] = 0;
           
            j = parent2[j];
            assert(j>0);
        }
    }
}

double BPP_C::log_f_Xz(vector<bool> visited, vector<vector<vec>>& lambda_tmp, bool neut, double propos, BPP& bpp)  //calculate marginal likelihood P(X|Z,I)
{
    
    
    // 1. sending the lambda msg from leaves bottom up through the network
    vec tmp_diag = zeros<mat>(bpp.num_base);
    mat transition(bpp.num_base,bpp.num_base);
    for(vector<int>::iterator it= internal_nodes.begin(); it < internal_nodes.end(); it ++ ) //int s=S; s<=root; s++)
    {
        int s = *it;
        if(visited[s]) continue; //visited s includes missing s
        
        int* p = children2[s];
        
        for(int cc=0;cc<2;cc++)
        {
            int chi = p[cc];
            assert(chi != -1);
            
            if(missing[chi]) continue;
            
            if(chi == bpp.moveroot) //rate doesn't change!!, may include other outgroup; doesn't matter, since the branch length is long, small rate change doesn't infludence liklihood much
            {
                transition = log_cache_TM_null[chi];
            }
            else if(!neut){  // sample cons rate
                if(Z[chi] ==1) {
                    mat x = bpp.eigenvec;
                    tmp_diag  = exp(bpp.eigenval*distances2[chi]*propos);
                    x.each_col()%=tmp_diag;
                    transition = log(bpp.eigeninv * x);
                    
                }
                else if(Z[chi] == 2) transition = log_cache_TM_neut[chi];
                else transition = log_cache_TM_null[chi];
            }
            else{
                if( Z[chi] ==2) {
                    
                    mat x = bpp.eigenvec;
                    tmp_diag  = exp(bpp.eigenval*distances2[chi]*propos);
                    x.each_col()%=tmp_diag;
                    transition = log(bpp.eigeninv * x);
                }
                else if (Z[chi] ==1) transition = log_cache_TM_cons[chi];
                else transition = log_cache_TM_null[chi];
            }
            
            
            for(int g=0;g<GG;g++)
                if(Tg[g][chi]!=bpp.num_base) lambda_tmp[g][s] += BPP::log_multi(transition,lambda_tmp[g][chi]);
            
            
        }
    }

    

    
    // 2. processing the distribution of root species
    double result = 0;
    for(int g=0;g<GG;g++)
    {
        if(!visited[root]) lambda_tmp[g][root] += log_pi;
        result += BPP::log_exp_sum(lambda_tmp[g][root]);
    }
    //if(c==0) cout << lambda_tmp[0][N-1] <<endl;
    return(result);
}

double BPP_C::sample_rate(int resZ, double old_rate, bool neut, vector<bool> visited, double & loglik_old, BPP& bpp, int M, bool adaptive, double adaptive_factor)
{  //element number ,n or c, #MH steps
    //MH to sample rate
    
    double scale_adj;
    double r,cur_r = old_rate, proposal, MH_ratio,loglik_new ,prior_a,prior_b;
    
    if(neut) {prior_a = bpp.nprior_a; prior_b = bpp.nprior_b;} else {prior_a = bpp.cprior_a; prior_b = bpp.cprior_b;}
    
    if(visited[root]) {
   //     return(old_rate);
        // sample from prior
        if(resZ!=0) old_rate = gsl_ran_gamma(RNG,prior_a,prior_b);
        return(old_rate);
//        return(ratio);  // originally for acceleration
//        if(neut)
//        {
//            return(1);
//        }else{
//            return(ratio);
//        }
    }
    
    
    vec tmp_diag = zeros<mat>(bpp.num_base);
    vector<vector<vec>> lambda_tmp = vector<vector<vec>> (GG, vector<vec>(N,zeros<vec>(bpp.num_base)));
    
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++)  // only copy visited[s]
    {
        int s = *it;
        if(!visited[s])
        {
            
            continue;
        }
        
        for(int g=0;g<GG;g++)
        {
            
            lambda_tmp[g][s] = lambda[g][s];
            
        }
    }
    
    
    
    for(int mm =0; mm<M;mm++)
    {
        r = gsl_rng_uniform(RNG);
        
        
        if(neut) {
            proposal =gsl_ran_gamma(RNG,cur_r/prop_n,prop_n);
            if(bpp.ropt == 1 && proposal <  bpp.nlb ) //trace_c_rate[m]) 0.6
            {
                return(cur_r);
            }else if(bpp.ropt == 2 && proposal < trace_c_rate[m])
            {
                 return(cur_r);
            }
        }
        else{
            proposal =gsl_ran_gamma(RNG,cur_r/prop_c,prop_c);
            if(bpp.ropt == 1 && proposal > bpp.cub) //trace_n_rate[m+1]) 1
            {
                return(cur_r);
            }else if(bpp.ropt == 2 && proposal > trace_n_rate[m+1])
            {
                return(cur_r);
            }
        }
        
        // if(m>0)cout << cur_r << "; " << proposal <<"; ";
        
        loglik_new = log_f_Xz(visited, lambda_tmp, neut, proposal,bpp);  //P(X|Z), lambda not changed, lambda_tmp changed
        

        if(neut) MH_ratio = loglik_new -loglik_old + log(gsl_ran_gamma_pdf(proposal,prior_a,prior_b)) - log(gsl_ran_gamma_pdf(cur_r,prior_a,prior_b)) + log(gsl_ran_gamma_pdf(cur_r,proposal/prop_n,prop_n)) - log(gsl_ran_gamma_pdf(proposal,cur_r/prop_n,prop_n));
        else  MH_ratio = loglik_new -loglik_old + log(gsl_ran_gamma_pdf(proposal,prior_a,prior_b)) - log(gsl_ran_gamma_pdf(cur_r,prior_a,prior_b)) + log(gsl_ran_gamma_pdf(cur_r,proposal/prop_c,prop_c)) - log(gsl_ran_gamma_pdf(proposal,cur_r/prop_c,prop_c));
        
       
     //cout << cur_r << "; " << proposal <<"; " << loglik_old  << "; " << loglik_new << ";" <<loglik_new -loglik_old<< ";" << MH_ratio << endl;
       
        
        if(log(r) < MH_ratio)
        {
            cur_r = proposal;
            loglik_old = loglik_new;
            //getUpdate some lambdas
            for(vector<int>::iterator it= internal_nodes.begin(); it < internal_nodes.end(); it ++ )  // int s=S; s<=root;s++)
            {
                int s = *it;
                if(visited[s]) continue; // || parent2[s] == -1
                for(int g=0;g<GG;g++)
                {                    
                    lambda[g][s] = lambda_tmp[g][s];
                    
                }
            }
            
            
        }
        
        // reset lambda_tmp to zero
        if(mm<M-1)
        {
            for(vector<int>::iterator it= internal_nodes.begin(); it < internal_nodes.end(); it ++ )//int s=S; s<=root;s++)
            {
                int s = *it;
                if(visited[s]) continue; //|| parent2[s] == -1
                for(int g=0;g<GG;g++)
                {
                    lambda_tmp[g][s].fill(0);
                }
            }
        }
        
    }
    
    
    
    
    if(old_rate!= cur_r)
    {
        
        for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++)
        {
            int s = *it;
            if(s==bpp.moveroot) continue; //not update rate for outgroup! s==42||, conserved rate still changing
            
            tmp_diag  = exp(bpp.eigenval*distances2[s]*cur_r);
            mat x = bpp.eigenvec;
            x.each_col()%=tmp_diag;
            
            if(neut) log_cache_TM_neut[s] = log(bpp.eigeninv * x); //transpose Q
            else log_cache_TM_cons[s] = log(bpp.eigeninv * x);
        }
        
        //getEmission_ambig(false);
        if(neut)
        {
            accept_n_rate +=1;
        }else{
            accept_c_rate +=1;
            
        }
        
    }
    
    //adaptive MCMC
    if (adaptive) {
        if ((m+1) % adaptive_freq == 0) {
            if(neut)
            {
                if((double) accept_n_rate/adaptive_freq > 0.44)
                    scale_adj = exp(min(adaptive_factor, 1 / sqrt((m+1) / adaptive_freq)));
                else if ((double) accept_n_rate/adaptive_freq < 0.23)
                    scale_adj = exp(-min(adaptive_factor, 1 / sqrt((m+1) / adaptive_freq)));
                else
                    scale_adj = 1;
                prop_n = prop_n * scale_adj;
                accept_n_rate = 0;
            }else{
                if((double) accept_c_rate/adaptive_freq > 0.44)
                    scale_adj = exp(min(adaptive_factor, 1 / sqrt((m+1) / adaptive_freq)));
                else if ((double) accept_c_rate/adaptive_freq < 0.23)
                    scale_adj = exp(-min(adaptive_factor, 1 / sqrt((m+1) / adaptive_freq)));
                else
                    scale_adj = 1;
                prop_c = prop_c * scale_adj;
                accept_c_rate = 0;
            }
        }
    }
    

    
    return(cur_r);
}





vector<int> BPP_C::Update_Z_subtree(int num_base)  //whether cal prob_back again
{
    
    getEmission(num_base);
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++)
    {
        int s =*it;
        //if(missing[s] ) continue; //&& s<S, for all nodes! don't update log_prob_back for missing nodes
        
        log_prob_back[s] = log_emission[s];
        
        if(fixZ[s] ==1)
        {
            log_prob_back[s][2] = -INFINITY; //log_prob_back[s][0] = -INFINITY;
        }else if(fixZ[s] ==2)  // no use!
        {
            log_prob_back[s][0] = -INFINITY;  //log_prob_back[s][1] = -INFINITY;
        }
    }
    
    
    //message passing from bottom to top
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++)
    {
        int s = *it;
        int p = parent2[s];
        
        log_prob_back[p] += BPP::log_multi2(log_TM_Int[s], log_prob_back[s]);  //matrix * vector
        //cout << log_prob_back[p] <<endl;
    }
    

    vector<int> changedZ;

    
    //update Z from top to bottom Z[N] = 0
    int old;
    vec log_trans_p, trans_p;
    
    for(vector<int>::iterator it =nodes.end()-2;it>=nodes.begin();it--)  //exclude root, doesn't include missing nodes
    {
        
        int s = *it;
        int p = parent2[s];
        
        //if(fixZ[s] == 1) continue;
        //if(fixZ[s] == 2 && (missing[s] && s<S)) continue;
        
        old = Z[s];
        if(Z[p]!=2)
        {
            log_trans_p =log_TM_Int[s].unsafe_col(Z[p]) + log_prob_back[s];
            trans_p = BPP::log_sample(log_trans_p);
            
            //cout << exp(log_y).t()<<endl;
            //cout << trans_p.t()<<endl;
            
            unsigned int n[3];
            gsl_ran_multinomial(RNG, 3, 1,trans_p.memptr(),n);
            int nn;
            for(nn=0; nn<3;nn++)
            {
                if(n[nn]>0) break;
            }
            
            Z[s] =nn;
            if(nn>2) {
                cout<< "Sample Z error" <<endl;
            }
            
        }
        else{
            Z[s] =2;
            
        }
        
        if(old != Z[s]) changedZ.push_back(s);
        
        // cout << log_prob_back[s] <<endl;
        // cout<<trans_p<<endl;
        
    }
    return(changedZ);
}


//// updating Z, return a vector of Z is changed, from top to bottom, for all the g
//vector<int>  BPP_C::Update_Z(int num_base)
//{
//
//    vector<int> changedZ;
//    
//    
//    for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++) //N-1
//    {
//        int s = *it;
//        int p = parent2[s];
//    
//        double emission =0, emission2 = 0, emission3 =0 ;
//        if(missing[s])
//        {
//            //confined log_prob_back in initMCMC for s<S; otherwise log_emission ==0
//            if(s>=S) log_prob_back[s].fill(0);
//            continue;
//        }
//       
//        //sum over all g
//        for(int g=0;g <GG;g++)
//        {
//            if(Tg[g][s] == num_base) continue;
//            
//            if(Tg[g][s] ==-1){
//                
//                emission += ambiguousS_null[g][s][Tg[g][p]];
//                emission2 += ambiguousS_cons[g][s][Tg[g][p]];
//                emission3 += ambiguousS_neut[g][s][Tg[g][p]];
//                
//            }else{
//                emission += log_cache_TM_null[s].at(Tg[g][s],Tg[g][p]);
//                emission2 += log_cache_TM_cons[s].at(Tg[g][s],Tg[g][p]);
//                emission3 += log_cache_TM_neut[s].at(Tg[g][s],Tg[g][p]);
//            }
//        }
//        log_prob_back[s][0] =  emission; log_prob_back[s][1] =  emission2; log_prob_back[s][2] =  emission3;
//
//    }
//    
//
//    //message passing from bottom to top
//    for(vector<int>::iterator it = nodes.begin(); it < nodes.end()-1;it++)
//    {
//        int s = *it;
//        int p = parent2[s];
//       
//        log_prob_back[p] += BPP::log_multi2(log_TM_Int[s], log_prob_back[s]);  //matrix * vector
//        //cout << log_prob_back[p] <<endl;
//    }
//
//
//    //update Z from top to bottom Z[N] = 0
//    vec log_trans_p, trans_p;
//    for(vector<int>::iterator it = nodes.end() -2; it >= nodes.begin();it--) //N-2
//    {
//
//        int s = *it; //also sample missing Z!!
//        int p = parent2[s];
//        
//        int old = Z[s];
//        
//        if(Z[p]!=2)
//        {
//            log_trans_p = log_TM_Int[s].col(Z[p]);
//       
//
//            log_trans_p =log_trans_p + log_prob_back[s];
//            trans_p = BPP::log_sample(log_trans_p);
//
//            //cout << exp(log_y).t()<<endl;
//            //cout << trans_p.t()<<endl;
//
//            unsigned int n[3];
//           
//            gsl_ran_multinomial(RNG, 3, 1,trans_p.memptr(),n);
//            int nn;
//            for(nn=0; nn<3;nn++)
//            {
//                if(n[nn]>0) break;
//            }
//           
//
//            Z[s] = nn;
//
//            if(nn>2) {
//                throw runtime_error("Sample Z error");
//            }
//        }
//        else{
//            Z[s] = 2;
//        }
//
//        if(old != Z[s]) changedZ.push_back(s);
//
//       // cout << log_prob_back[s] <<endl;
//       // cout<<trans_p<<endl;
//
//    }
//    return(changedZ);
//
//}


vector<int>  BPP_C::Move_Z(int & propConf, int & revConf, int & changeZ){
    
    // propose new Z
    vector<int> propZ = vector<int>(N, 0);
    vector<int> increaseZ;
    vector<int> decreaseZ;
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end() - 1; it ++ )
    {
        int s = *it;
        int p = parent2[s];
        
        propZ[s] = Z[s] ;
        if(Z[s] == 2 && Z[p] == 1)
        {
            decreaseZ.push_back(s);
        }
        else if(Z[s] == 1 && Z[p] == 0)
        {
            int* cc = children2[s];
            if(cc[0]  == 2 || cc[1]  == 2) continue;
            decreaseZ.push_back(s);

        }
        else if(fixZ[s]!=1 && Z[s] == 1 && Z[p] == 1)
         {
             int* cc = children2[s];
             if(s<S || (cc[0]  == 2 && cc[1]  == 2)){
                 increaseZ.push_back(s);
             }
         }
        else if(fixZ[s]!=0 && Z[s] == 0) //= moveroot
        {
            int* cc = children2[s];
            if(cc[0]  == 1 && cc[1]  == 1){
                increaseZ.push_back(s);
            }
        }
    }
    
    int deN = decreaseZ.size();
    int inN = increaseZ.size();
    
    int totN = deN + inN;
    propConf = totN;
    
    int ind = rand() % (deN + inN);
    
    // compute reverse prob: propZ -> Z
    if(ind < deN)
    {
        changeZ = decreaseZ[ind];
        propZ[changeZ] -= 1;
        int p = parent2[changeZ];
        //int* cc = children2[s];
        if(Z[changeZ] == 1)
        {
            // s : 1 -> 0, p can't 0 -> 1
            if(fixZ[p]!=0 && Z[children2[p][0]] == 1 && Z[children2[p][1]] == 1 ) //p!= moveroot (and root)
            {
                totN -= 1;
            }
            // s : 1 -> 0, c can't 1 -> 2
            if(changeZ>=S)
            {
                for(int i  = 0; i < 1;i ++)
                {
                    int c = children2[changeZ][i];
                    if(fixZ[c] == 1) continue;
                    if(c<S || (Z[children2[c][0]]  == 2 && Z[children2[c][1]]  == 2))
                    {
                        totN--;
                    }
                    
                    if(c<S || (Z[children2[c][0]]  == 1 && Z[children2[c][1]]  == 1))
                    {
                        totN++;
                    }
                }
            }
            
        }
        else if(Z[changeZ] == 2)
        {
            // s : 2 -> 1, p can't 1 -> 2
            if( Z[parent2[p]] == 1 && Z[children2[p][0]]  == 2 && Z[children2[p][1]]  == 2 && fixZ[p]!=1){
                totN--;
            }
            else if(Z[parent2[p]] == 0 && propZ[children2[p][0]]  == 1 && propZ[children2[p][1]]  == 1) // s : 2 -> 1, p can 1 -> 0
            {
                totN++;
            }
            // s : 2 -> 1, p can be 2 -> 1
            if(changeZ >= S)
            {
                totN += 2;
            }
        }
    }else{
        changeZ = increaseZ[ind - deN];
        int p = parent2[changeZ];
        propZ[changeZ] += 1;
        if(Z[changeZ] == 1){
            // s : 1 -> 2, p can't 1 -> 0
            if(Z[parent2[p]] == 0 && Z[children2[p][0]] == 1 && Z[children2[p][1]] == 1)
            {
                totN--;
            }else if(Z[parent2[p]] == 1 && propZ[children2[p][0]] == 2 && propZ[children2[p][1]] == 2 && fixZ[p]!=1)
            {
                totN++;
            }
            // s : 1 -> 2, c can't 2 -> 1
            if(changeZ >= S)
            {
                totN -= 2;
            }
        }else if(Z[changeZ] == 0){
            // s : 0 -> 1, p can 0 -> 1
            if(fixZ[p]!=0 && Z[children2[p][0]] == 1 && Z[children2[p][1]] == 1 ) //p != moveroot
            {
                totN++;
            }
            if(changeZ >= S)
            {
                // s : 0 -> 1, c can't 1 -> 0
                // s : 0 -> 1, c can 1 -> 2
                for(int i  = 0; i < 1;i ++)
                {
                    int c = children2[changeZ][i];
                    if( c<S || (Z[children2[c][0]]  == 1 && Z[children2[c][1]]  == 1)) totN --;
                    if(fixZ[changeZ]!=1 &&( c<S || (Z[children2[c][0]]  == 2 && Z[children2[c][1]]  == 2))) totN++;
                }
            }


        }

    }
    
    revConf = totN;
    
    return(propZ);
    
}



double BPP_C::Update_f_Xz(vec log_pi, int num_base, vector<int>& Z, vector<mat> & log_cache_TM_neut, vector<mat> & log_cache_TM_cons, vector<bool> & visited)
{
    double result =0;
    // 1. sending the lambda msg from leaves bottom up through the network
    for(vector<int>::iterator it = internal_nodes.begin(); it < internal_nodes.end(); it++)//int s=S; s<=root; s++)
    {
        int s = *it;
        if(visited[s] || missing[s]) continue;
        for(int g=0;g<GG;g++)
        {
            
            if(Tg[g][s]==num_base) continue;
            
            int* p = children2[s];
            //lambda[g][s].fill(0);
            
            for(int cc=0;cc<2;cc++)
            {
                int chi = p[cc];
                assert(chi != -1);
                if(missing[chi] || Tg[g][chi]==num_base) continue;
                
                if(Z[chi] ==2) lambda[g][s] +=  BPP::log_multi(log_cache_TM_neut[chi],lambda[g][chi]);
                else if (Z[chi] ==0) lambda[g][s] +=  BPP::log_multi(log_cache_TM_null[chi],lambda[g][chi]);
                else lambda[g][s] +=  BPP::log_multi(log_cache_TM_cons[chi],lambda[g][chi]);
            }
        }
        
    }
    
    // 2. processing the distribution of root species
    for(int g=0;g<GG;g++)
    {
        if(!visited[root]) lambda[g][root] += log_pi;
        result += BPP::log_exp_sum(lambda[g][root]);
    }
    
    for(vector<int>::iterator it = nodes.begin(); it < nodes.end(); it++)
    {
        if(missing[*it]) result += log_emission[*it][Z[*it]];// for missing
    }
    
    result += prior_Z_subtree(Z);
    return(result);
    
}


    




