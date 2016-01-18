 /*A Wait Free Red Black Tree*/



#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "insertion.h"
#include "deletion.h"
#include "read.h" 
#include "window.h"
#include "invariant.h"
#include "helping.h"
//#include "windowcase2.h"
//#include "gc.h"
//#include "update.h"

#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
/* Note: stdio is thread-safe */
#endif

/*
 * Useful macros to work with transactions. Note that, to use nested
 * transactions, one should check the environment returned by
 * stm_get_env() and only call sigsetjmp() if it is not null.
 */
 
#define RO                              1
#define RW                              0

#define DEFAULT_DURATION                10000
#define DEFAULT_TABLE_SIZE              10
#define DEFAULT_NB_THREADS              10
#define DEFAULT_SEED                    0
#define DEFAULT_SEARCH_FRAC             0.0
#define DEFAULT_INSERT_FRAC             0.5
#define DEFAULT_DELETE_FRAC             0.5
#define DEFAULT_READER_THREADS         	0
#define DEFAULT_KEYSPACE1_SIZE          100
#define DEFAULT_KEYSPACE2_SIZE          1000000000
#define KEYSPACE1_PROB           1.0
#define KEYSPACE2_PROB           0.0

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

/* ################################################################### *
 * GLOBALS
 * ################################################################### */

 


 
static volatile AO_t stop;


long total_insert = 0;



timespec t;
/* STRUCTURES */


enum{
Front,
Back
};


long blackCount = -1;
long leafNodes = 0;



/* ################################################################### *
 * Correctness Checking
 * ################################################################### */
 
void check_black_count(node_t * root, int count){
        // 2. That the number of black nodes from the root to the leaf are same
	
	if(is_node_marked(root->opData)){
		std::cout << "Node marked, but not passive__" << root << std::endl;
		exit(0);
	}
	int status = extract_status_from_opdata(root->opData);
	if(status != DONE && status != FREE){
		std::cout << "Status is not free or done" << std::endl;
		exit(0);
	}	
	
        if(root->color == BLACK ){

            count++;
        }
	
	
	
	node_t * lChild = (node_t *)get_child(root->lChild);
	node_t * rChild = (node_t *)get_child(root->rChild);
	
	if(lChild == root || rChild == root){
		std::cout << "Error. Child pointer is the same as node" << std::endl;
		exit(0);
	}
	
	
        if(lChild != NULL){

            check_black_count(lChild, count);
        }

        
        if(rChild != NULL){
            check_black_count(rChild, count);
        }
        


        if(lChild == NULL && rChild == NULL){
            if(root->key != -1){
            	leafNodes++;
            }
            if(blackCount == -1){
                blackCount = count;

            }
            else if(blackCount != count){
            	 std::cout << "Black Counts do not match!!!__" << blackCount << "__"<< count << std::endl;
            	 exit(0);
            }
        
        }
        
}


void check_red_property(node_t * root){
	// 1. To check that no red node has a red child
	
	node_t * lChild = (node_t *)get_child(root->lChild);
	if(lChild != NULL){
	
		if((root->color == RED) && (lChild->color == RED)){
			std::cout << "ERROR!!! DOUBLE RED CONDITION EXISTS IN THE TREE!!";
			exit(0);
		}
		check_red_property(lChild); 
	}
	
	node_t * rChild = (node_t *)get_child(root->rChild);
	if(rChild != NULL){
		
	
		if((root->color == RED) && (rChild->color == RED)){
			std::cout << "ERROR!!! DOUBLE RED CONDITION EXISTS IN THE TREE!!";
			exit(0);
		}
		check_red_property(rChild); 
		
	}
	
} 
 
long in_order_visit(node_t * root){
	
	if((node_t *)get_child(root->lChild) != NULL){
	
		long lKey = in_order_visit((node_t *)get_child(root->lChild));
		if(lKey > root->key){
			std::cout << "Lkey is larger!!__" << lKey << "__ " << root->key << std::endl;
			std::cout << "Sanity Check Failed!!" << std::endl;
			
		}
	}
	
	
	if((node_t *)get_child(root->rChild) != NULL){
	
		long rKey = in_order_visit((node_t *)get_child(root->rChild));
		if(rKey <= root->key){
			std::cout << "Rkey is smaller!!__" << rKey << "__ " << root->key <<  std::endl;
			std::cout << "Sanity Check Failed!!" << std::endl;
		}
	}
	return (root->key);
}





/************************************************************************************************/

/************************************************************************************************/

node_t * perform_external_node_replacement(thread_data_t * data, int pid, node_t * curParent, long key){
	
	
	// curParent is parent of an external node.
	
	node_t * curLeaf = NULL;
	if(key <= curParent->key){
		curLeaf = (node_t *)get_child(curParent->lChild);
	}
	else{
		curLeaf = (node_t *)get_child(curParent->rChild);
	}
	
	//valrec_t * oldrec = (curDNode->valData);
	
	long curKey = curLeaf->key;
	
	if(key == curKey){
		
		return NULL;
	}
	
	
	node_t * newInt = (node_t *)get_new_node(data);
	
	
	
	node_t * newLeaf = (node_t *)get_new_node(data);
	
				
				(newInt->opData) = NULL;
				
				
				
				(newLeaf->opData) = NULL;
				
				//newRCDNode->op = operation;
				//newRCDNode->parent = NULL; // will be correctly set during next window operation
				//newRCDNode->lChild = NULL;
				//newRCDNode->rChild = NULL;
				newLeaf->color = BLACK;
				newLeaf->key = key;
				//newRCDNode->rootCount = 0;
				
				
				if(key <= curKey){
					//newLCDNode->valData = (newrec);
					//newRCDNode->valData = (oldrec);	
					newInt->key = key;
					newInt->lChild = create_child_word(newLeaf,OFIN);
					newInt->rChild = create_child_word(curLeaf,OFIN);
				}
				else{
					newInt->key = curKey;
					newInt->rChild = create_child_word(newLeaf,OFIN);
					newInt->lChild = create_child_word(curLeaf,OFIN);
					
				}
				
				newInt->parent = curParent;
				
				/*node_t * par = curLeaf->parent;
				
				

				if((node_t *)get_child(par->lChild) == curLeaf){
					par->lChild = create_child_word(newInt, INTERNAL,OFIN);
				}
				else if((node_t *)get_child(par->rChild) == curLeaf){
					par->rChild = create_child_word(newInt, INTERNAL,OFIN);
				}
				else{
					std::cout << "Parent pointer not assigned correctly for existing leaf" << std::endl;
					exit(0);
				}*/
				
				//curDNode->valData = NULL;
				//curLeaf->lChild = create_child_word(newLC,LEAF,OFIN);
				
				//curLeaf->rChild = create_child_word(newRC,LEAF,OFIN);
				
					data->hasInserted = key;
					return newInt;
				
	
}



node_t * insert(thread_data_t * data, int pid, long key, node_t * wRoot, node_t * wRootChild, AO_t casField, oprec_t * O){
	
	  node_t * bot = (node_t *)map_word_to_bot_address(1);
	 	  
	  int currentWindowSize = 3;
	  
	  bool case2flag = false;
	  
  	currentWindowSize = 3;
  	 
  	node_t * wRootChildCopy = make_window_copy(data, wRootChild, key, currentWindowSize,  wRoot, casField, O);
  	
  	if(wRootChildCopy == (node_t *)map_word_to_bot_address(1)){
  		return (bot);
  	}
  	
	#ifdef DEBUG
  	if(wRootChildCopy == O->sr->addresses[1]){	
  		std::cout << "Error copying insert window" << std::endl;
  		exit(0);	
  	}
	#endif
  	node_t * lastNode = set_last_node(data, wRootChildCopy, currentWindowSize, key);
  	
  	while(data->madeDecision == false){	
	 
	
	// std::cout << "2" << std::endl;
	

		if(wRoot == data->prootOfTree){
		if(wRootChildCopy->color == RED){
			wRootChildCopy->color = BLACK;
		}
		// make children of root black if they are both red
		
		node_t * lc = (node_t *)get_child(wRootChildCopy->lChild);
		//dataNode_t * lcd = NULL;
		
		//if(lc!= NULL){
		//	 lcd = (dataNode_t *)extract_dnode_from_ptrnode(*(lcp));
		//}	
		node_t * rc = (node_t *)get_child(wRootChildCopy->rChild);
		//dataNode_t * rc = NULL;
		//if(rcp!= NULL){
		//	rcd = (dataNode_t *)extract_dnode_from_ptrnode(*(rcp));
		//}
		if(lc != NULL && rc != NULL){
			if(lc->color == RED && rc->color == RED){
				lc->color = BLACK;
				rc->color = BLACK;
				
			}	
		}
		
		
		
	}
	
	
	//valrec_t * wrootkvp = windowRootDNode->valData;
	// case 1: Tree is empty
	if((wRootChildCopy->key == -1)){ 
		
			// std::cout << "3" << std::endl;
		
		wRootChildCopy->key = key;
		
		data->hasInserted = key;
		
		wRootChildCopy->move = NULL;
		//check_window(data, windowRootDNode, currentWindowSize, key);
		
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		//iif(result1 == 1)
		//	wRoot->cseq = 380;
		
		
		if(result0 == 1){						
			
			if(data->hasInserted != 0){
				data->numInsert++;
				data->hasInserted = 0;
			}
				//update_rtable_entries(data, windowRootPNode, map_datanode_address_to_hplist_entry(windowRootDNode));
			//flush_window_on_dcas_success(data,pid);
			return NULL;
		} 
		else{
			//std::cout << "CAS FAILURE" << std::endl;
			//exit(0);
			//free_window_on_dcas_failure(data,pid);
			data->hasInserted = 0;
			return NULL;
		}		
		
	}
	
	// tree is not empty
	
	
	
	if((is_external_node(wRootChildCopy))){
			
			long curKey = wRootChildCopy->key;
			
			if(curKey == key){
				//std::cout << "4" << std::endl;
				//add_to_hp_list(data, map_record_address_to_hplist_entry(currec), false,0);
				//windowRootDNode->op = -1;
				//windowRootDNode->rootCount = 1463;
			 	//AO_double_t localRootPtr = (casField);
			 	//AO_double_t newRootPtrNode;
			 	wRootChildCopy->move = NULL;
			 //	check_window(data, windowRootDNode, currentWindowSize, key);
			 	
			 		//newRootPtrNode.AO_val1 = combine_record_move_and_procid(currec, pid);
			//newRootPtrNode.AO_val2 = combine_dnode_and_flag(windowRootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
			int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		//int result = atomic_cas_full1(data,windowRootPNode,casField, combine_dnode_status_ptrnode(windowRootDNode,DONE));
		//if(result1== 1)
		//	wRoot->cseq = 435;	
				if(result0 == 1){	
					
					/*if(try_fast_update(data, currec) == 0){
							slow_update(data, currec);	
					}		
					//remove_from_hp_list(data,map_record_address_to_hplist_entry(currec));
					
						data->hasInserted = 0;*/
					//update_rtable_entries(data, windowRootPNode, map_datanode_address_to_hplist_entry(windowRootDNode));
						
				//	flush_window_on_dcas_success(data,pid);
					return(NULL);
		
				} 
				else{
				//std::cout << "CAS FAILURE1" << std::endl;
				//exit(0);
				//remove_from_hp_list(data, map_record_address_to_hplist_entry(currec));
				//free_window_on_dcas_failure(data,pid);
					data->hasInserted = 0;
					return NULL;
				}
				
				
			}
			else{
				// std::cout << "5" << std::endl;
				//node_t * dnode = perform_external_node_replacement(data,pid, wRootChildCopy, key);
				node_t * newLeaf = (node_t *)get_new_node(data);
				newLeaf->key = key;
				node_t * newInt = (node_t *)get_new_node(data);
				long curKey = wRootChildCopy->key;
				//std::cout << "curkey__" << curKey << std::endl;
				if(key <= curKey){
					//newLCDNode->valData = (newrec);
					//newRCDNode->valData = (oldrec);	
					newInt->key = key;
					newInt->lChild = create_child_word(newLeaf,OFIN);
					newInt->rChild = create_child_word(wRootChildCopy,OFIN);
				}
				else{
					newInt->key = curKey;
					newInt->rChild = create_child_word(newLeaf,OFIN);
					newInt->lChild = create_child_word(wRootChildCopy,OFIN);
					
				}
				data->hasInserted = key;
			 	newInt->move = NULL;
			 	newInt->color = RED;
			 	//	newInt->mover = 1485;
			 	//	}
			 	//	else{
			 	//		windowRootDNode->next = combine_move_status_datanode(NULL,COMPLETED);
			 	//	}
			 	
			 		//newRootPtrNode.AO_val1 = combine_move_and_procid(NULL, pid);
			//newRootPtrNode.AO_val2 = combine_dnode_and_flag(windowRootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
	//	check_window(data, windowRootDNode, currentWindowSize, key);
		//int result = atomic_cas_full1(data,windowRootPNode, casField, combine_dnode_status_ptrnode(windowRootDNode,DONE));
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(newInt,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(newInt,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		//if(result1 == 1)
		//	wRoot->cseq = 506;	
				if(result0 == 1){
						
					// cas success
					/*if(rec != NULL){
						if(try_fast_update(data, rec) == 0){
							// fast update failed. Perform slow update
							slow_update(data, rec);	
						}
						//remove_from_hp_list(data, map_record_address_to_hplist_entry(rec));		
					}
						*/
					if(data->hasInserted != 0){
						data->numInsert++;
						data->hasInserted = 0;
					}
					//update_rtable_entries(data, windowRootPNode, map_datanode_address_to_hplist_entry(windowRootDNode));
					//flush_window_on_dcas_success(data,pid);
					return(NULL);
		
				} 
				else{	
					//std::cout << "CAS FAILURE2" << std::endl;
					//exit(0);
					// cas failure. Some other node executed window
					/*if(rec != NULL){
						remove_from_hp_list(data, map_record_address_to_hplist_entry(rec));
					}*/
					//free_window_on_dcas_failure(data,pid);
					return NULL;
				}		
						
						
				
			}
			
			return NULL;
	}
	//std::cout << "6" << std::endl;
	/// case 3: Multiple nodes exist in the tree.	
	// Here we need to find out which one of the insertion cases are satisfied
	node_t * nextWRoot = get_next_node_on_access_path(data, pid, wRoot, wRootChild, wRootChildCopy, key, currentWindowSize, casField, case2flag,O);
	
	if(nextWRoot == NULL && data->madeDecision == false){
		currentWindowSize++;
		
		
  		
  		
  		lastNode = extend_current_window(data, lastNode, key, currentWindowSize, wRoot, casField,O);
  		if(lastNode == bot){
  			return (bot);
  		}
  		
  		continue;
	}
	


  if(nextWRoot != NULL){	
	//if(is_parent_of_external_node(nextWRoot,key)){
   		// std::cout << "7" << std::endl;
		// external node reached.
		// need to insert node and rebalance 
		node_t * dnode = perform_external_node_replacement( data, pid, nextWRoot, key);
               
                if(dnode != NULL){
		//dnode->seq = O->seq;	
		
		if(key <= nextWRoot->key){
			nextWRoot->lChild = create_child_word(dnode, OFIN);
		}
		else{
			nextWRoot->rChild = create_child_word(dnode, OFIN);
		}	
		dnode->color = RED;
                //dnode->parent = nextWRoot;
                if(nextWRoot == wRootChildCopy){
                	dnode->parent = NULL;
                }
		node_t * finalRootNode = balance_after_insertion(data, dnode, wRootChildCopy);
		
		// INSERTING HERE
		if(finalRootNode != NULL){
			//std::cout << "8" << std::endl;
		//if(finalRootNode == wRootChild)
			//std::cout << "Error__592" << std::endl;
			finalRootNode->move = NULL;
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(finalRootNode,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(finalRootNode,ONEED));
		}
		
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
			//if(result1 == 1)
				//wRoot->cseq = 625;
				AO_t lchildWord;
				AO_t rchildWord;		
				//if(child == LEFT){
					lchildWord = wRoot->lChild;
				//}
				//else{
					rchildWord = wRoot->rChild;
				//}	
				if((node_t *)get_child(lchildWord) == O->sr->addresses[1] || (node_t *)get_child(rchildWord) == O->sr->addresses[1]){
					std::cout << "Error..614" << std::endl;
				}

				if(result0 == 1){	
					if(data->hasInserted != 0){
						data->numInsert++;
						data->hasInserted = 0;
					}
					return(NULL);
		
				} 
				else{
					data->hasInserted = 0;
					return NULL;
				}				
		}
		else{
			//std::cout << "9" << std::endl;
			dnode->move = NULL;
		
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(dnode,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(dnode,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		//if(result1 == 1)
		//	wRoot->cseq = 671;
		//int result = atomic_cas_full1(data,windowRootPNode, casField, combine_dnode_status_ptrnode(windowRootDNode,DONE));
				if(result0 == 1){						
					// cas success
					if(data->hasInserted != 0){
						data->numInsert++;
						data->hasInserted = 0;
					}
					//update_rtable_entries(data, windowRootPNode, map_datanode_address_to_hplist_entry(windowRootDNode));
					//flush_window_on_dcas_success(data,pid);
					return(NULL);
		
				} 
				else{
					//std::cout << "CAS FAILURE3" << std::endl;
				//exit(0);
						//free_window_on_dcas_failure(data,pid);
					data->hasInserted = 0;
					return NULL;
					
				}	
		
			}
			return NULL;	
		}
		
			
			//add_to_hp_list(data, map_record_address_to_hplist_entry(rec), false,0);
			//std::cout << "10" << std::endl;
			//wRootChildCopy->mover = 1700;
			wRootChildCopy->move = NULL;
			//AO_double_t localRootPtr = (casField);
			//AO_double_t newRootPtrNode;
			
			//	newRootPtrNode.AO_val1 = combine_record_move_and_procid(rec, pid);
		//newRootPtrNode.AO_val2 = combine_dnode_and_flag(windowRootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
		//check_window(data, windowRootDNode, currentWindowSize, key);
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		//if(result1 == 1)
		//	wRoot->cseq = 720;
		//int result = atomic_cas_full1(data,windowRootPNode, casField, combine_dnode_status_ptrnode(windowRootDNode,DONE));
				if(result0 == 1){						
					// cas success
							
					
					data->hasInserted = 0;
					//update_rtable_entries(data, windowRootPNode, map_datanode_address_to_hplist_entry(windowRootDNode));
					//flush_window_on_dcas_success(data,pid);
					return NULL;
		
				} 
				else{
					// cas failure. Some other node executed window
					//std::cout << "CAS FAILURE4" << std::endl;
			//exit(0);
					//remove_from_hp_list(data, map_record_address_to_hplist_entry(rec));
					//free_window_on_dcas_failure(data,pid);
					data->hasInserted = 0;
					return NULL;
				}	
					

		
	}
  //}
  	
  }

	
} 

/*************************************************************************************************/

int perform_one_window_operation(thread_data_t* data, node_t* pRoot, oprec_t * O, int sucid){
	


	/*if(pid != data->id){
		// helping
		
		data->helped++;
	}*/
	
	AO_t pRootContents = (pRoot->opData);
	AO_t pRootContents0 = pRootContents;
	if(!is_node_marked(pRootContents)){
		pRootContents0 = add_mark_flag(pRootContents);
	}
	node_t * oldLoc = (node_t *)extract_position_from_oprecord(O->windowLoc);
	AO_t pRootContents3 = pRoot->opData;
	if( oldLoc != pRoot && oldLoc != NULL){
		if((pRootContents3 == pRootContents) || (pRootContents3 == pRootContents0)){
			AO_t oldLocOpData = oldLoc->opData;
			if((extract_status_from_opdata(oldLocOpData) == DONE) && ((oprec_t *)extract_oprec_from_opdata(oldLocOpData) == O)){
				//help_within_window(data, O, oldLoc, oldLocOpData,814);
				update_oprecord(data, O, oldLoc, oldLocOpData,776);
			}
			
		}
		return NULL;
	}
	
	//dataNode_t * dRoot = (dataNode_t *)extract_dnode_from_ptrnode(pRootContents);
	//pRootContents = (pRoot->opData);
	data->lastCase = 822;
	
	int temp = 824;
	if((extract_status_from_opdata(pRootContents) == PLOCK) && ((oprec_t *)extract_oprec_from_opdata(pRootContents) == O)){
	// Make sure that operation is still pending by checking that the child status is OFIN
	int child = extract_child_from_opdata(pRootContents);
		AO_t childWord;
		if(child == LEFT){
			childWord = pRoot->lChild;
		}
		else{
			childWord = pRoot->rChild;
		}
		temp = 825;
	if(get_child_status(childWord) == ONEED){
		// operation has been completed. Change node status to DONE
		temp = 838;
		int result = atomic_cas_full1(data, pRoot, pRootContents, combine_oprec_status_child_opdata(O,DONE,child));
	#ifdef DEBUG
		
		if(result == 1){
				pRoot->cseq = 804;
			node_t * cnode = (node_t *)get_child(childWord);
			if(cnode == O->sr->addresses[1])
				std::cout << "Bug is here" << std::endl;	
		}
	#endif	
			
	}
	else{
	//CHECK!!
	oldLoc = (node_t *)extract_position_from_oprecord(O->windowLoc);
	//if( oldLoc != pRoot && oldLoc != NULL){
		/*if(get_child_status(pRootContents)){
			std::cout << "WindowLOC STILL NOT UPDATED!!" << std::endl;
			exit(0);
		}*/
	//}	
	// operation is still pending
	data->lastCase = 789;
	temp = 858;
	node_t * bot = (node_t *)map_word_to_bot_address(1);
	data->helpCount = 0;
	long key = map_word_to_key(O->op);

	unsigned opn = map_word_to_operation(O->op);	
	//AO_t pChild;
	node_t * pRootChild = (node_t *)get_child(childWord);
	/*if(key <= pRoot->key ){
		pChild = pRoot->lChild;
	}
	else{
		pChild = pRoot->rChild;
	}	
	pRootChild = (node_t *)get_child(pChild); 
	if(!is_node_marked(pRootContents)){
		pRootContents0 = add_mark_flag(pRootContents);
	}
	else{
		pRootContents0 = pRootContents;
	}*/
	
	pRootContents3 = pRoot->opData;
	
	if((pRootContents3 != pRootContents) && (pRootContents3 != pRootContents0)){
		//Operation has moved
		return NULL;
	}
	data->lastCase = 827;	
	int status = extract_status_from_opdata(pRootChild->opData);
	if(status != SLOCK){
		std::cout << "Status is not slock!!__" << status << std::endl;	
		exit(0);
	}
	
	
	//dataNode_t * newRootDNode = NULL;
	node_t * nextRoot1 = NULL;
		if(opn == 2){
		
			temp = 878;
			 nextRoot1 = insert(data, O->pid, key, pRoot, pRootChild, pRootContents , O);
			 /*if(nextRoot1 == bot){
			 	
			 	free_window_on_dcas_failure(data,pid);
			 }*/
			
		
	
		}
	
		else if(opn == 1){
		
		
				temp = 892;
			//data->lastdel = 0;
		
	
    
	    		bool case2flag = false;
	    		
	    		
				//auto del_start = std::chrono::high_resolution_clock::now();	

			node_t * wRootChildCopy = make_delete_window_copy(data, pRootChild, key, pRoot, pRootContents,O);
			
			//	if(wRootChildCopy == O->sr->addresses[1]){
			//		std::cout << "Error in copying delete window" << std::endl;
			//		exit(0);	
			//	}
			if(wRootChildCopy == (node_t *)map_word_to_bot_address(1)){
			
				//free_window_on_dcas_failure(data,pid);
				data->madeDecision = false;
				//return (NULL);
			}
			// delete operation
			else{
				if(pRoot == data->prootOfTree){
					bool flag = false;
					if(wRootChildCopy->color == BLACK){
						flag = true;
					}
		
					node_t * lc = (node_t *)get_child(wRootChildCopy->lChild);
					//dataNode_t * lcd = NULL;
		
					//if(lcp!= NULL){
					//	 lcd = (dataNode_t *)extract_dnode_from_ptrnode(*(lcp));
					//}	
					node_t * rc = (node_t *)get_child(wRootChildCopy->rChild);
					//dataNode_t * rcd = NULL;
					//if(rc!= NULL){
					//	rcd = (dataNode_t *)extract_dnode_from_ptrnode(*(rcp));
					//}
					if(lc != NULL && rc != NULL){
						if(lc->color == BLACK && rc->color == BLACK && flag){

							wRootChildCopy->color = RED;
						}	
				
					}	
				}
		
				//auto del_start = std::chrono::high_resolution_clock::now();	
				
				nextRoot1 = get_next_node_on_delete_access_path(data, key, pRoot, wRootChildCopy, pRootChild, pRootContents, O, case2flag);
				
				
				
			}
			
			//blackCount = -1;
			//check_black_count(data->rootOfTree, 0);
			
		} 
	
	  }
	}
	
	 AO_t pRootContents1 = (pRoot->opData);
	 //dataNode_t * dNow = (dataNode_t *)extract_dnode_from_ptrnode(pRootContents);
		 
	 Word posWord = O->windowLoc;
	 
	 
	 if((extract_status_from_opdata(pRootContents1) == DONE) && ((oprec_t *)extract_oprec_from_opdata(pRootContents1) == O)){
	 
		//help_within_window(data, O, pRoot, pRootContents1,temp);
		update_oprecord(data, O, pRoot, pRootContents1,950);
	 	
	 	
	 }
	
	else if((extract_status_from_opdata(pRootContents1) == PLOCK) && ((oprec_t *)extract_oprec_from_opdata(pRootContents1) == O)){
		std::cout << "OPERATION STILL PENDING AFTER HELPING" << std::endl;
		exit(0);
	} 
	
	data->helpCount = 0;
	
	data->madeDecision = false;
	return temp;
		
}


/* ################################################################### *
 * Protocol Definitions
 * ################################################################### */
 
 void upgrade_announcement(thread_data_t * data, oprec_t * O){
 	
 /*	std::ofstream ofile;
		
		char fname[50];
		char title[20] = "file"; 
		 sprintf(fname,"%s_%d",title,data->id);
		ofile.open(fname, std::ios::app);*/
		
 	
 	
 	seekrec_t * R = O->sr;
 	
 	AO_t E = combine_oprec_status_child_opdata(O,ANNOUNCE,R->isLeft);
 	AO_t G = combine_oprec_status_child_opdata(O,PLOCK,R->isLeft);
 	
 	int result = atomic_cas_full(&R->addresses[0]->opData,E,G);
 	if(result != 1){
 		// node may have been marked.
 		AO_t E1 = combine_oprec_status_child_mark_opdata(O,ANNOUNCE,R->isLeft);
 		AO_t G1 = combine_oprec_status_child_mark_opdata(O,PLOCK,R->isLeft);
 		
 		int result2 = atomic_cas_full(&R->addresses[0]->opData,E1,G1);
 		//if(result2 == 1){
 			//ofile << O->seq << "_a_" << O->pid << "\n";
 		//}
 	}
 	else{
 		//ofile << O->seq << "_b_" << O->pid << "\n";
 	}
 	//ofile.close();
 	
 }
 
 
void release_announcement(thread_data_t * data, oprec_t * O){

	seekrec_t * R = O->sr;	
	
 	AO_t E = combine_oprec_status_child_opdata(O,ANNOUNCE,R->isLeft);
 	AO_t H = combine_oprec_status_child_opdata(O,ABORT,R->isLeft);
 	
 	int result = atomic_cas_full(&R->addresses[0]->opData,E,H);
 	if(result != 1){
 		// node may have been marked.
 		AO_t E1 = combine_oprec_status_child_mark_opdata(O,ANNOUNCE,R->isLeft);
 		AO_t H1 = combine_oprec_status_child_mark_opdata(O,ABORT,R->isLeft);
 		
 		int result2 = atomic_cas_full(&R->addresses[0]->opData,E1,H1);
 		if(result2 == 1){
 			data->lastCase2 = 938;
 		}
 		else{
 			data->lastCase2 = 941;
 		}
 		
 	}
 	else{
 		data->lastCase2 = 946;
 	}
}
 
 bool lock_invariant_path(thread_data_t * data, oprec_t * O){
 	
 	seekrec_t * R = O->sr;
 	// obtain first secondary lock
 	AO_t newWord1 = combine_oprec_status_child_opdata(O,SLOCK,LEFT); // default is left
	AO_t newWord2;
		int st = extract_status_from_opdata(R->contents[1]);	
	int result1 = atomic_cas_full(&R->addresses[1]->opData, R->contents[1], newWord1);
	//R->addresses[1]->slocker = 1000 + st;
	if(result1 != 1){
		// CAS failure. Determine if node has already been locked on behalf of the same operation.
		AO_t W1 = (R->addresses[1]->opData);
		if((oprec_t *)extract_oprec_from_opdata(W1) != O){
			// different operation	
			data->lastCase1 = 953;
			return false;
		}
		else{
			if(extract_status_from_opdata(W1) == ABORT){
				// operation aborted.
				data->lastCase1 = 960;
				return false;
			}
		}
		
		
	}
	
	// acquire any remaining secondary lock
	
	if(R->addresses[2] != NULL){
		//if(is_external_node(R->addresses[2])){
		//	std::cout << "Leaf in ipath00!!" << std::endl;
		//	exit(0);
		//}
		newWord2 = combine_oprec_status_child_opdata(O,SLOCK,LEFT);
		st = extract_status_from_opdata(R->contents[2]);	
		int result1 = atomic_cas_full(&R->addresses[2]->opData, R->contents[2], newWord2);
		//R->addresses[2]->slocker = 1100+st;
		if(result1 != 1){
			// CAS failure. Determine if node has already been locked on behalf of the same operation.
			AO_t W1 = (R->addresses[2]->opData);
			if((oprec_t *)extract_oprec_from_opdata(W1) != O){
				// different operation. Release acquired secondary lock	
				AO_t newWord1Abort = combine_oprec_status_child_opdata(O,ABORT,LEFT);
				atomic_cas_full(&R->addresses[1]->opData, newWord1, newWord1Abort);
				data->lastCase1 = 981;
				return false;
			}
		}
	}
	
	if(R->addresses[3] != NULL){
		//if(is_external_node(R->addresses[3])){
		//	std::cout << "Leaf in ipath11!!" << std::endl;
		//	exit(0);
	//	}
	
		AO_t newWord3 = combine_oprec_status_child_opdata(O,SLOCK,LEFT);
		st = extract_status_from_opdata(R->contents[3]);	
		int result1 = atomic_cas_full(&R->addresses[3]->opData, R->contents[3], newWord3);
		//R->addresses[3]->slocker = 1200+st;
		if(result1 != 1){
			// CAS failure. Determine if node has already been locked on behalf of the same operation.
			AO_t W1 = (R->addresses[3]->opData);
			if((oprec_t *)extract_oprec_from_opdata(W1) != O){
				// different operation. Release acquired secondary lock	
				AO_t newWord2Abort = combine_oprec_status_child_opdata(O,ABORT,LEFT);
				atomic_cas_full(&R->addresses[2]->opData, newWord2, newWord2Abort);
				
				atomic_cas_full(&R->addresses[1]->opData, newWord1, newWord2Abort);
				
				data->lastCase1 = 981;
				return false;
			}
		}
	}
 		
 	return true;
 }
 
 
bool validate(thread_data_t * data, oprec_t * O){
	
	seekrec_t * R = O->sr;
	
	// first check if any nodes are owned.
	
	for(int i = 3; i >= 0; i--){
		//data->lastCase = 0;
		if(R->contents[i] != NULL){
				unsigned status = extract_status_from_opdata(R->contents[i]);
			if((status == ANNOUNCE) || (status == PLOCK) || (status == SLOCK)){
				//dataNode_t * dnode = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[i]);
				oprec_t * N = (oprec_t *)extract_oprec_from_opdata(R->contents[i]);
				int help = help_during_validation(data, N , R->addresses[i], R->contents[i],1148);
				
				/*if((data->lastAborted == R->addresses[i]) && (data->lastAbortedContents == R->contents[i]) && (status != PLOCK)){
					data->samehelpcount++;
					//dataNode_t * rdnode = (dataNode_t *)extract_dnode_from_ptrnode(*(dnode->opData->sr->addresses[0]));
					//dataNode_t * rdnode1 = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[0]);
					if(data->samehelpcount == 3){
						std::cout << "Aborted by same opn" << std::endl;
						exit(0);
					}
					data->lasthelp = help;
				//}
				//else{
					//data->lastAborted = R->addresses[i];
					//data->lastAbortedContents = R->contents[i];
					//data->lastAbortedOprec = (oprec_t *)extract_oprec_from_opdata(data->lastAbortedContents);
					//data->lasthelp = help;
					//data->lastAborted1 = R->addresses[0];
					//data->lastAbortedContents1 = R->contents[0];
					//data->lasthelp = help;
				//}
				
				//data->lastAborted = R->addresses[i];
				//data->lastAbortedContents = R->contents[i];
				data->lastCase = i+99;*/
				data->case1++;
				return false;
			}
		}
	}
	
	// next check if any nodes are marked.
	
	for(int i = 3; i >= 0; i--){
		if(is_node_marked(R->contents[i])){
			node_t * marker = R->addresses[i]->markedRoot;
			if(marker != NULL){
				//dataNode_t * dnode = (dataNode_t *)extract_dnode_from_ptrnode(R->llContents);
				
				//int help = help_during_validation(data, (oprec_t *)extract_oprec_from_opdata(R->llContents), R->llAddress, R->llContents,1182);
				//data->lasthelp = help+10000;
				//data->lastCase = 2;
				AO_t state = marker->opData;
				oprec_t * N = (oprec_t *)extract_oprec_from_opdata(state);
				unsigned status = extract_status_from_opdata(state);
				if(status == PLOCK){
					int help = help_during_validation(data, N, marker, state, 1155);
					data->lasthelp = help;
				}
				/*else{
					//data->lasthelp = 1158;
					if(data->lastAborted == marker){
						std::cout << "marked node is active" << std::endl;
						exit(0);
					}
					else{
						data->lastAborted =marker;
						data->lastAbortedContents = state;
						data->lastAbortedOprec = (oprec_t *)extract_oprec_from_opdata(marker->opData);
						data->lastCase = 1160;
					}
				} */
				
			}
			//else{
				data->case2++;
			//}
			return false;	
		}
	}
	
	// Finally, check if any node has a DONE status, and update position of operation
	
	for(int i = 3; i >= 0; i--){
		if(R->contents[i] != NULL){
			unsigned status = extract_status_from_opdata(R->contents[i]);
			if((status == DONE)){
				//dataNode_t * dnode = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[i]);
				oprec_t * N = (oprec_t *)extract_oprec_from_opdata(R->contents[i]);
				
				//help_within_window(data, N, R->addresses[i], R->contents[i],1199);
				update_oprecord(data, N, R->addresses[i], R->contents[i],1213);
				
				
			}
		}
	}
	

	/*if(R->contents[0] == 0){
		AO_t lchildWord;
		AO_t rChildWord;	
		//if(R->isLeft == LEFT){
			lchildWord = R->addresses[0]->lChild;
		//}
		//else{
			rChildWord = R->addresses[0]->rChild;
		//}
		if(get_child_status(lchildWord) == ONEED || get_child_status(rChildWord) == ONEED){
			std::cout << "Fishy here" << std::endl;
			exit(0);
		}
	}	*/
	// Create copies of data nodes.
	/* for(int i = 2; i >=0; i--){
		if(R->contents[i] != NULL){
			R->dcopies[i] = (dataNode_t *)get_new_dataNode(data);
			dataNode_t * dnode = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[i]);
			
			R->dcopies[i]->key = dnode->key;
			R->dcopies[i]->lChild = dnode->lChild;
			R->dcopies[i]->rChild = dnode->rChild;
			//R->dcopies[i]->las = dnode->parent;
			R->dcopies[i]->color = dnode->color;
			R->dcopies[i]->opData = O;
			R->dcopies[i]->move = NULL;
		//	R->dcopies[i]->creator = 1001;
		}
		
	} */
	
	return true;
}




int inject(thread_data_t * data, oprec_t * O){
	
	seekrec_t * R = O->sr;
	
	// invariant path has more than one node.
	
	AO_t newWord1 = combine_oprec_status_child_opdata(O,ANNOUNCE,R->isLeft);
	#ifdef DEBUG
	if(R->addresses[0] == data->prootOfTree && R->isLeft == RIGHT){
		std::cout << "This is happening" << std::endl;
		exit(0);
	}
	#endif
	
	int result1 = atomic_cas_full(&R->addresses[0]->opData, R->contents[0], newWord1);
	if(result1 == 1){
		// Announcement successful
		if(lock_invariant_path(data, O)){
			// successfully locked invariant path.
			upgrade_announcement(data,O);
			return 1;
		}
		else{
			data->lastCase0 = 1132;
			release_announcement(data, O);
			return 3; // CANNOT! reuse O and R
		}
	}
	else{
		// Announcement failed.
		data->lastCase0 = 1139;
		return 2; // can reuse O and R
	}
}


void execute_operation(thread_data_t* data, oprec_t * O){

  unsigned pid = data->id;
  


  AO_t curLoc = O->windowLoc;	
  
  Word operation = O->op;
  unsigned opn = map_word_to_operation(operation);
  
  node_t * nextRoot;
  int iter = 0;
  
  
  while( extract_position_from_oprecord(curLoc) != NULL){
 	 iter++;
  	
  
  	nextRoot = (node_t *)extract_position_from_oprecord(curLoc);
  	
  	perform_one_window_operation(data, nextRoot, O, 1070);
  	if(iter > 1000000){
		AO_t word = nextRoot->opData;
		oprec_t * N = (oprec_t*)extract_oprec_from_opdata(word);
		AO_t word1 = O->sr->addresses[0]->opData;
		int status = extract_status_from_opdata(word1);
		oprec_t * M = (oprec_t *)extract_oprec_from_opdata(word1);
  		std::cout << "Iterations Exceeded__1302" << std::endl;
  		exit(0);
  	}
  	
  	  curLoc = O->windowLoc;	

 }

	
 
 // status is completed. 
 
 
  
}




/* ################################################################### *
 * BARRIER
 * ################################################################### */



void barrier_init(barrier_t *b, int n)
{
  pthread_cond_init(&b->complete, NULL);
  pthread_mutex_init(&b->mutex, NULL);
  b->count = n;
  b->crossing = 0;
}

void barrier_cross(barrier_t *b)
{
  pthread_mutex_lock(&b->mutex);
  /* One more thread through */
  b->crossing++;
  /* If not all here, wait */
  if (b->crossing < b->count) {
    pthread_cond_wait(&b->complete, &b->mutex);
  } else {
    pthread_cond_broadcast(&b->complete);
    /* Reset for next time */
    b->crossing = 0;
  }
  pthread_mutex_unlock(&b->mutex);
}

/* ################################################################### *
 * TEST
 * ################################################################### */

void pre_insert(thread_data_t * data, long key){
	oprec_t * O = (oprec_t *)get_new_opRecord(data);
    	O->op = map_key_to_insert_operation(key);
 	O->pid = data->id;
 	//O->state = combine_position_status_oprecord(d->rootOfTree);
 	//O->seq = data->seqNo++;
 	
 	seekrec_t * R = (seekrec_t *)get_new_seekRecord(data);
 	
 	O->sr = R;
 	
 	int iter = 0;
 	
	//std::cout << "Insert" << std::endl;

 	while(true){
 		iter++;
 	
		seek(data, O, key,R);
		if(R->contents[3] != 0){
       			std::cout << "Whats this?" << std::endl;
       			exit(0);
       		}
	
	
		if(R->leafKey == key){
		
			// key already present in the tree. Terminate
	
					
		
			
			break;
		
		}
		else{
			
			if(!validate(data, O)){
				// validation failed. Restart seek.
				std::cout << "Validation Failed" << std::endl;
				exit(0);
				continue;
			}
			
			// validation successful. Inject operation into the tree at R->addresses[0]
			
			O->windowLoc = combine_position_init_oprecord(R->addresses[0],PRINCIPAL);
			
			if(inject(data, O)){
				// operation successfully injected. Execute sequence of window transactions.
				 	
				execute_operation(data, O);
				//data->seqNo++;
				//std::cout << "Insert" << std::endl;
				//blackCount = -1;
				//check_black_count(data->rootOfTree, 0);
			}
			else{
				// injection failed. Restart seek	
				continue;
			}
			
		
		
			
			break;
	
		}
	
	}
 	

}


void *testRW(void *data)
{
   long value;
   double action;
   double ksSelect;
   Word key;
   thread_data_t *d = (thread_data_t *)data;
   
   //sTable_t * ST = *(d->stable);
        
  /* Wait on barrier */
  barrier_cross(d->barrier);
  

while (stop == 0) {
	d->case2 = 0;
	//d->samehelpcount = 0;
	d->case1 = 0;
     // determine what we're going to do
     action = (double)rand_r(&d->seed) / (double) RAND_MAX;
   
  	key = rand_r(&d->seed) % (d->keyspace1_size);
	      while(key == 0){
	      	key = rand_r(&d->seed) % (d->keyspace1_size);
	      } 
   
   	
     if (action <= d->search_frac)
     {
   
  	
   
   	//auto search_start = std::chrono::high_resolution_clock::now();
    	
    	
    	
    	
    	search(d, key);
    	
	
	
	//auto search_end = std::chrono::high_resolution_clock::now();
	
	//auto search_dur = std::chrono::duration_cast<std::chrono::microseconds>(search_end - search_start);
	
	//d->tot_read_time += search_dur.count();
	//d->tot_reads++;
     
     }
    
          
    else if (action > d->search_frac && action <= (d->search_frac + d->insert_frac))
     {
	// Insert Operation
	
	//auto ins_start = std::chrono::high_resolution_clock::now();
	//std::cout << "Insert" << std::endl;
	//valrec_t * reskvp = NULL;
	long leafKey = update_search(d, key); 
	
	
	if(leafKey != key){
	
	oprec_t * O = (oprec_t *)get_new_opRecord(d);
    	O->op = map_key_to_insert_operation(key);
 	O->pid = d->id;
 	//O->state = combine_position_status_oprecord(d->rootOfTree);
 	
 	
 	seekrec_t * R = (seekrec_t *)get_new_seekRecord(d);
 	
 	// create datanode copies, whose contents are populated during validation
 	
 	//R->dcopies = (dataNode_t *)malloc(2*sizeof(dataNode_t));
 	/*for(int k = 0; k < 2; k++){
 		R->dcopies[k] = (dataNode_t *)malloc(sizeof(dataNode_t));	
 	}*/
 	
 	//O->seq = d->seqNo++;
 	O->sr = R;
 	
 	int iter = 0;
 	int lastCase = 0;
 	
 	node_t * ld1 = NULL;
 	node_t * ld0 = NULL;
 	d->lastAborted = NULL;
 	
	while(true){
		iter++;
		if(iter > 100000){
			std::cout << "Iterations exceeded11__" << lastCase << "__" << d->case1  << "__" << d->case2 << std::endl;
			exit(0);
			
		}
		
		
		for(int i1 = 0; i1 < 4; i1++){
			R->addresses[i1] = NULL;
			R->contents[i1] = 0;
		}
		
		seek(d, O, key, R);
	
	
	
		if(R->leafKey == key){
		
			// key already present in the tree. Terminate
	
			/*if(try_fast_update(d, reskvp) == 0){
				// fast update failed. Perform slow update
			
				slow_update(d, reskvp);	
			}*/
		
		
		
			//auto ins_end = std::chrono::high_resolution_clock::now();
	
			//auto ins_dur = std::chrono::duration_cast<std::chrono::microseconds>(ins_end - ins_start);
		
			//d->tot_update_time += ins_dur.count();
			//d->tot_update_count++;
			break;
		
		}
		else{
			
			if(!validate(d, O)){
				// validation failed. Restart seek.
				lastCase = 1;
				
				//TODO: FIX THIS!!
				//if(R->addresses[] != NULL){
				//	ld1 = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[1]);
				//}
				
				//if(R->addresses[] != NULL){
				//	ld0 = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[0]);
				//}
				
				continue;
			}
			
			// validation successful. Inject operation into the tree at R->addresses[0]
			
			O->windowLoc = combine_position_init_oprecord(R->addresses[0],PRINCIPAL);
			
			int res = inject(d, O);
			
			if(res == 1){
				// operation successfully injected. Execute sequence of window transactions.
				// std::cout << "insert" << std::endl; 	
				
				
				execute_operation(d, O);
				//d->seqNo++;
				//blackCount = -1;
				//check_black_count(d->rootOfTree, 0);	
				
			}
			else if(res == 2){
				// injection failed. Restart seek
				//Can reuse O and R
				for(int i1 = 0; i1 < 4; i1++){
					R->addresses[i1] = NULL;
					R->contents[i1] = 0;
				}
					
				lastCase = 2;	
				continue;
			}
			else {
				// New O and R
				O = (oprec_t *)get_new_opRecord(d);
				O->op = map_key_to_insert_operation(key);
				O->pid = d->id;
 	
 	
			 	R = (seekrec_t *)get_new_seekRecord(d);
			 	//O->seq = d->seqNo++;
 				O->sr = R;
			 	continue;
			}
			
		
		
			//auto ins_end = std::chrono::high_resolution_clock::now();
	
			//auto ins_dur = std::chrono::duration_cast<std::chrono::microseconds>(ins_end - ins_start);
			
			//if(O->txns == 0){
			//	std::cout << "0 txns executed" << std::endl;
			//}
			
			//d->tot_ins_wt += O->txns;
					
			//d->tot_insert_time += ins_dur.count();
			//d->tot_insert_count++;
			break;
	
		}
	
	}
	
      }
     // else{
      		//	auto ins_end1 = std::chrono::high_resolution_clock::now();
	
			//auto ins_dur1 = std::chrono::duration_cast<std::chrono::microseconds>(ins_end1 - ins_start);
		
			//d->tot_update_time += ins_dur1.count();
			//d->tot_update_count++;
      //}
	
	  
     }
     
     else
     {
    
        // Delete Operation
	
	//std::cout << "Delete" << std::endl;
	
	int flag = 0;
	
	//auto del_start = std::chrono::high_resolution_clock::now();
	
	long leafKey = update_search(d, key);
       
       
       if(leafKey == key){
       	// execute expensive operation
       oprec_t * O = (oprec_t *)get_new_opRecord(d);
    	O->op = map_key_to_delete_operation(key);
 	O->pid = d->id;
// 	O->state = combine_position_status_oprecord(d->rootOfTree, INPROGRESS);
 	//O->seq = d->seqNo++;
 	
 	seekrec_t * R = (seekrec_t *)get_new_seekRecord(d);
 	
 	//R->dcopies = (dataNode_t *)malloc(3*sizeof(dataNode_t));
 	/*for(int k = 0; k < 3; k++){
 		R->dcopies[k] = (dataNode_t *)malloc(sizeof(dataNode_t));	
 	}*/
 	
 	int lastCase = 0;
 	int iter = 0;
 	
 	O->sr = R;
 	
	while(true){	
		
		
		
		iter++;
		
		if(iter > 100000){
			std::cout << "Iterations exceeded12__" << lastCase << "__" << d->case1  << "__" << d->case2 << std::endl;
			exit(0);
			
		}
		
		for(int i1 = 0; i1 < 4; i1++){
			R->addresses[i1] = NULL;
			R->contents[i1] = 0;
		}
		
		seek(d, O, key,R);
       	
       
       		
        
		if(R->leafKey == key){
	       		flag = 1;
	       		
	       		if(!validate(d, O)){
				// validation failed. Restart seek.
				lastCase = 1;
				continue;
			}
			
			
				// validation successful. Inject operation into the tree at R->addresses[0]
			
			// verify that the operation does satisfy the invariant
			
			/*for(int i = 2; i >= 0; i--){
				if(R->addresses[i] != NULL){
					dataNode_t * rootDNode = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[i]);
					
					if(rootDNode->color != RED && R->addresses[i] != d->rootOfTree){
						std::cout << "Node is not red" << std::endl;
						exit(0);
					}
					else{
						break;
					}
				}
			}*/
			
			O->windowLoc = combine_position_init_oprecord(R->addresses[0],PRINCIPAL);
			int res = inject(d, O);
			if(res == 1){
				// operation successfully injected. Execute sequence of window transactions.
				//std::cout << "Delete" << std::endl;
				//auto del_start = std::chrono::high_resolution_clock::now();	
				//auto del_end2 = std::chrono::high_resolution_clock::now();
				//auto del_dur2 = std::chrono::duration_cast<std::chrono::microseconds>(del_end2 - del_start);
				//d->tot_slowdel_time += del_dur2.count();
				//d->tot_slowdel_count++;
				 	
				//auto del_start = std::chrono::high_resolution_clock::now();	 	
				 	
				execute_operation(d, O);
				
				//auto del_end2 = std::chrono::high_resolution_clock::now();
				///auto del_dur2 = std::chrono::duration_cast<std::chrono::microseconds>(del_end2 - del_start);
				///d->tot_slowdel_time += del_dur2.count();
				//d->tot_slowdel_count++;
				
				
				//blackCount = -1;
				//check_black_count(d->rootOfTree, 0);	
			}
			else if(res == 2){
				// reuse O and R	
				lastCase = 2;
				for(int i1 = 0; i1 < 3; i1++){
					R->addresses[i1] = NULL;
					R->contents[i1] = 0;
				}
				
				// injection failed. Restart seek	
				continue;
			}
			else{
				lastCase = 3;
				O = (oprec_t *)get_new_opRecord(d);
				O->op = map_key_to_delete_operation(key);
				O->pid = d->id;
 	
 	
			 	R = (seekrec_t *)get_new_seekRecord(d);
			 	//O->seq = d->seqNo++;
 				O->sr = R;
 				continue;
			}	
	
		
			
		
		}
	
		//auto del_end = std::chrono::high_resolution_clock::now();
		//auto del_dur = std::chrono::duration_cast<std::chrono::microseconds>(del_end - del_start);
	
		if(flag == 1){
			
			
			
			//d->tot_del_wt += O->txns;
			
			//d->tot_slowdel_time += del_dur.count();
			//d->tot_slowdel_count++;	
			break;
	
		}
	  	
		else{
	
			//d->tot_fastdel_time += del_dur.count();
			//d->tot_fastdel_count++; 
			break;	
		}
	}
	
    }
    
     
     
   }  
     d->ops++;
	//blackCount = -1;
  	// check_black_count((node_t*)get_child(d->prootOfTree->lChild), 0);	
	
  }
	

  return NULL;
}

void catcher(int sig)
{
  printf("CAUGHT SIGNAL %d\n", sig);
}

int main(int argc, char **argv)
{
  struct option long_options[] = {
    // These options don't set a flag
    {"help",                      no_argument,       NULL, 'h'},
    {"table-size",                required_argument, NULL, 't'},
    {"duration",                  required_argument, NULL, 'd'},
    {"num-threads",               required_argument, NULL, 'n'},
    {"seed",                      required_argument, NULL, 's'},
    {"search-fraction",           required_argument, NULL, 'r'},
    {"insert-update-fraction",    required_argument, NULL, 'i'},
    {"delete-fraction",           required_argument, NULL, 'x'},
    {"keyspace1-size",             required_argument, NULL, 'k'},
    {"keyspace2-size",             required_argument, NULL, 'l'},
    {"gc-node-threshold",             required_argument, NULL, 'g'},
    {"reader-threads",             required_argument, NULL, 'a'},
    {NULL, 0, NULL, 0}
  };

//mTable_t** table;
//mTable_t* itable;

//sTable_t** stable;
//sTable_t* istable;

  int i, c;
  char *s;
  unsigned long inserts, deletes, actInsert, actDelete;
  thread_data_t *data;
  pthread_t *threads;
  pthread_attr_t attr;
  barrier_t barrier;
  struct timeval start, end, end1;
  struct timespec timeout;
  
  struct timespec gc_timeout;
  
  int duration = DEFAULT_DURATION;
  
  int tablesize = DEFAULT_TABLE_SIZE;
  int nb_threads = DEFAULT_NB_THREADS;
  double search_frac = DEFAULT_SEARCH_FRAC;
  double insert_frac = DEFAULT_INSERT_FRAC;
  double delete_frac = DEFAULT_DELETE_FRAC;
  long keyspace1_size = DEFAULT_KEYSPACE1_SIZE;
  long keyspace2_size = DEFAULT_KEYSPACE2_SIZE;
  long gcThr = GC_NODE_THRESHOLD;
  int reader_threads = DEFAULT_READER_THREADS;
  int seed = DEFAULT_SEED;
  sigset_t block_set;
  

  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "ht:d:n:s:r:i:x:k:l:g:a:", long_options, &i);

    if(c == -1)
      break;

    if(c == 0 && long_options[i].flag == 0)
      c = long_options[i].val;

    switch(c) {
     case 0:
       /* Flag is automatically set */
       break;
     case 'h':
       printf("Double Ended Queue\n"
              "\n"
              "Usage:\n"
              "  deque [options...]\n"
              "\n"
              "Options:\n"
              "  -h, --help\n"
              "        Print this message\n"
              "  -t, --table-size <int>\n"
              "        Number of cells in the hash table (default=" XSTR(DEFAULT_TABLE_SIZE) ")\n"
              "  -d, --duration <int>\n"
              "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
              "  -n, --num-threads <int>\n"
              "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
              "  -s, --seed <int>\n"
              "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
	      "  -r, --search-fraction <int>\n"
              "        Number of search Threads (default=" XSTR(DEFAULT_SEARCH_FRAC) ")\n"
              "  -i, --insert-update-fraction <int>\n"
              "        Number of insert/update Threads (default=" XSTR(DEFAULT_INSERT_FRAC) ")\n"
              "  -x, --delete-fraction <double>\n"
              "        Fraction of delete operations (default=" XSTR(DEFAULT_DELETE_FRAC) ")\n"
              "  -a, --reader-threads <int>\n"
              "        Number of reader threads (default=" XSTR(DEFAULT_READER_THREADS) ")\n"
              "  -k, --keyspace-size <int>\n"
               "       Number of possible keys (default=" XSTR(DEFAULT_KEYSPACE1_SIZE) ")\n"
              "  -l, --keyspace-size <int>\n"
               "       Number of possible keys (default=" XSTR(DEFAULT_KEYSPACE2_SIZE) ")\n"
              "  -g, --gc <int>\n"
               "       garbage collection threshold (default=" XSTR(GC_NODE_THRESHOLD) ")\n"
         );
       exit(0);
     case 't':
       tablesize = atoi(optarg);
       break;
     case 'd':
       duration = atoi(optarg);
       break;
     case 'n':
       nb_threads = atoi(optarg);
       break;
     case 's':
       seed = atoi(optarg);
       break;
     case 'r':
       search_frac = atof(optarg);
       break;
     case 'i':
       insert_frac = atof(optarg);
       break;
     case 'x':
       delete_frac = atof(optarg);
       break;
     case 'a':
       reader_threads = atoi(optarg);
       break;  
    case 'k':
       keyspace1_size = atoi(optarg);
       break;
     case 'l':
       keyspace2_size = atoi(optarg);
       break;
     case 'g':
       gcThr = atoi(optarg);
       break;
     case '?':
       printf("Use -h or --help for help\n");
       exit(0);
     default:
       exit(1);
    }
  }

  assert(duration >= 0);
  assert(tablesize >= 2);
  assert(nb_threads > 0);
  assert(nb_threads < MAX_PROCESSES);
  assert(reader_threads < nb_threads);


	
  printf("Duration       : %d\n", duration);
  printf("Nb threads     : %d\n", nb_threads);
  printf("Seed           : %d\n", seed);
  printf("Search frac    : %f\n", search_frac); 	
  printf("Insert frac    : %f\n", insert_frac);
  printf("Delete frac    : %f\n", delete_frac);
  printf("Reader Threads : %d\n", reader_threads);
  printf("Update Threads : %d\n", (nb_threads - reader_threads));   
  printf("Keyspace1 size : %d\n", keyspace1_size);
  printf("GC Threshold   : %ld\n", gcThr);


  timeout.tv_sec = duration / 1000;
  timeout.tv_nsec = (duration % 1000) * 1000000;
  
  
  data = new thread_data_t[nb_threads];


  if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
    perror("malloc");
    exit(1);
  }

  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);


 
 node_t * pRoot = (node_t *)malloc(sizeof(node_t));
 
 node_t * newNode = (node_t *)malloc(sizeof(node_t));

pRoot->key = (keyspace1_size + 1);	 
 pRoot->color = BLACK;
 pRoot->opData = combine_oprec_status_child_opdata(NULL, FREE,LEFT); 
 pRoot->parent = NULL;
 pRoot->lChild = create_child_word(newNode,OFIN);
 pRoot->rChild = NULL;
 pRoot->move = NULL;

  newNode->key = -1;
  newNode->color = BLACK;
  newNode->opData = NULL;
  //newDNode->valData = NULL;
  newNode->parent = pRoot;
  newNode->lChild = NULL;
  newNode->rChild = NULL;
  newNode->move = NULL;
  
  
  stop = 0;

//Pre-populate Tree-------------------------------------------------
	int i1 = 0;
	data[i1].id = i1+1;
    data[i1].numThreads = nb_threads;
    data[i1].numInsert = 0;
    data[i1].numDelete = -1;
    data[i1].numActualInsert = 0;
    data[i1].numActualDelete = 0;
    data[i1].seed = rand();
    data[i1].search_frac = 0.0;
    data[i1].insert_frac = 1.0;
    data[i1].delete_frac = 0.0;
    data[i1].keyspace1_size = keyspace1_size;
    data[i1].keyspace2_size = keyspace2_size;
    //data[i1].table = &itable;
   // data[i1].stable = &istable;
   // data[i1].hplist = hplist;
   // data[i1].maxHpSize = 0;
   // data[i1].maxValidSize = 0;
    data[i1].hasInserted = 0;
    data[i1].ops = 0;
    
    data[i1].madeDecision = false;
    data[i1].prootOfTree = pRoot;
    //data[i1].gcInst = 0;
    data[i1].barrier = &barrier;
    data[i1].helpCount = 0;

	 data[i].tot_update_time = 0;
    data[i].tot_insert_time = 0;
    data[i].tot_update_count = 0;
    data[i].tot_insert_count = 0;

    data[i1].tot_fastdel_time = 0;
    data[i1].tot_slowdel_time = 0;
    data[i1].tot_fastdel_count = 0;
    data[i1].tot_slowdel_count = 0;
    data[i1].lastRootCount = 0;
  
    
    data[i1].boCount = 2;
    data[i1].shouldBackOff = false;
	data[i1].nextPid = 1;
	data[i1].nextSPid = 1;
  
    
    data[i1].fastCount = 0;
    data[i1].slowCount = 0;
  
    
    data[i1].readSequenceNumber = 1;
    data[i1].seqNo = 0;
   // data[i1].tot_ins_wt = 0;
   // data[i1].tot_del_wt = 0;

    	
    	 Word key;
    	
    	while(data[0].numInsert < (keyspace1_size/2)){
    		key = rand_r(&data[0].seed) % (keyspace1_size);
	        while(key == 0){
	      		key = rand_r(&data[0].seed) % (keyspace1_size);
	      	} 
    		pre_insert(&data[0], key);	
    		//std::cout << "Inserted__" << data[0].numInsert << std::endl;
    		//check_red_property((node_t*)get_child(pRoot->lChild));
  	// blackCount = -1;
  	// check_black_count((node_t*)get_child(pRoot->lChild), 0);
	//std::cout << "Leafnodes__" << leafNodes << std::endl;

	
	
	//leafNodes = 0;
   	//long rootkey =  in_order_visit((node_t*)get_child(pRoot->lChild));
    	}
    	int pre_inserts = data[0].numInsert;
    		//------------------------------------------------------------------- 
    		std::cout << "Finished pre-population" << std::endl; 
		//exit(0);
  
 
  barrier_init(&barrier, nb_threads + 1);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
 
    
  for (i = 0; i < nb_threads; i++) {

    data[i].id = i+1;
    data[i].numThreads = nb_threads;
    data[i].numInsert = 0;
    data[i].numDelete = -1;
    data[i].numActualInsert = 0;
    data[i].numActualDelete = 0;
    data[i].seed = rand();
    data[i].search_frac = search_frac;
    data[i].insert_frac = insert_frac;
    data[i].delete_frac = delete_frac;
    data[i].keyspace1_size = keyspace1_size;
    data[i].keyspace2_size = keyspace2_size;
   // data[i].table = &itable;
   // data[i].stable = &istable;
  //  data[i].hplist = hplist;
  //  data[i].maxHpSize = 0;
  //  data[i].maxValidSize = 0;
 // data[i].iretry = 0;
    data[i].hasInserted = 0;
    data[i].ops = 0;
    data[i].madeDecision = false;
    data[i].prootOfTree = pRoot;
   // data[i].gcInst = 0;
    data[i].barrier = &barrier;
    data[i].helpCount = 0;
   data[i].tot_update_time = 0;
    data[i].tot_insert_time = 0;
    data[i].tot_update_count = 0;
    data[i].tot_insert_count = 0;
    
    data[i].tot_fastdel_time = 0;
    data[i].tot_slowdel_time = 0;
    data[i].tot_fastdel_count = 0;
    data[i].tot_slowdel_count = 0;
    data[i].nextPid = 1;
    data[i].nextSPid = 1;
   /* data[i].tot_gc_time_sec = 0;
    data[i].tot_gc_time_usec = 0;
    data[i].gcCount = 0;*/
 //   data[i].tot_reads = 0;	
    data[i].lastRootCount = 0;
   
    
    data[i].boCount = 2;
    data[i].shouldBackOff = false;

    data[i].lastCase0 = 0;
    data[i].lastCase1 = 0;
    data[i].lastCase2 = 0;
    
    data[i].fastCount = 0;
    data[i].slowCount = 0;
 
    
    data[i].readSequenceNumber = 1;
    data[i].seqNo = 10000;
    data[i].lastAborted = NULL;
    data[i].lastAbortedContents = NULL;
    
 //   data[i].tot_ins_wt = 0;
  //  data[i].tot_del_wt = 0;
    
    
    	if (pthread_create(&threads[i], &attr, testRW, (void *)(&data[i])) != 0) {
    	  fprintf(stderr, "Error creating thread\n");
    	  exit(1);
    	}
 }
  
  pthread_attr_destroy(&attr); 

  /* Catch some signals */
 
  if (signal(SIGHUP, catcher) == SIG_ERR ||
      signal(SIGINT, catcher) == SIG_ERR ||
      signal(SIGTERM, catcher) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  /* Start threads */
    
    barrier_cross(&barrier);

  
  gettimeofday(&start, NULL);
  if (duration > 0) {
	nanosleep(&timeout, NULL);
	
  } 
  else {
    sigemptyset(&block_set);
    sigsuspend(&block_set);
    }
    
    	AO_store_full(&stop, 1);	
    gettimeofday(&end, NULL);
  
    
  /* Wait for thread completion */
  

   
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error waiting for thread completion\n");
      exit(1);
    }
  } 
  
  // CONSISTENCY CHECK
  
  
	
   leafNodes = 0;	
 
 std::cout << "Performing Sanity Check" << std::endl;
  
   check_red_property((node_t*)get_child(pRoot->lChild));
   blackCount = -1;
   check_black_count((node_t*)get_child(pRoot->lChild), 0);
   long rootkey =  in_order_visit((node_t*)get_child(pRoot->lChild));
 	
   unsigned long tot_inserts = 0;
   unsigned long tot_deletes = 0;
   unsigned long tot_ops = 0;
   unsigned long tot_reads = 0;
   double tot_read_time = 0;

   double tot_gc_time = 0;
   unsigned long tot_ptr_dealloc = 0;
   unsigned long tot_rec_dealloc = 0;
   unsigned long tot_data_dealloc = 0;
   unsigned long tot_ptr_free = 0;
   unsigned long tot_data_free = 0;
   unsigned long tot_rec_free = 0;
   unsigned long tot_gc_count = 0;
   
   double tot_update_time = 0;
   
   double tot_insert_time = 0;
   
   double tot_fastdel_time = 0;
   
   double tot_slowdel_time = 0;
   

   unsigned long tot_update_count = 0;
   unsigned long tot_insert_count = 0;
   
   unsigned long tot_fastdel_count = 0;
   unsigned long tot_slowdel_count = 0;
   
   unsigned long tnpc = 0;
   unsigned long tndc = 0; 		
   unsigned long tnrc = 0;
   
unsigned long trpc = 0;
unsigned long trdc = 0;
unsigned long trrc = 0;	
 
    unsigned long fdel = 0;
 
uint64_t tot_fastupdate_count = 0;
 
   uint64_t tot_slowupdate_count = 0;
   	uint64_t inscases = 0;
   	uint64_t retries = 0;
   	   	uint64_t tot_helped = 0;
uint64_t totinswin = 0;
uint64_t totdelwin = 0;

   for(int i = 0; i < nb_threads; i++){
   	tot_inserts += data[i].numInsert;
   	tot_deletes += data[i].numActualDelete;
   	/*tot_reads += data[i].tot_reads;
   	
   //	tot_read_time += (data[i].tot_read_time_sec * 1000000) + (data[i].tot_read_time_usec);
   //	tot_gc_time += (data[i].tot_gc_time_sec ) + (data[i].tot_gc_time_usec/1000000);
   	
   	tot_read_time += (data[i].tot_read_time);
   	
   	tot_update_time += (data[i].tot_update_time);
   	tot_insert_time += (data[i].tot_insert_time);
   	
   	tot_fastdel_time += (data[i].tot_fastdel_time);
   	tot_slowdel_time += (data[i].tot_slowdel_time);
   	
   	tot_update_count += data[i].tot_update_count; 
   	 tot_insert_count += data[i].tot_insert_count;
   	 
   	 tot_fastdel_count += data[i].tot_fastdel_count; 
   	 tot_slowdel_count += data[i].tot_slowdel_count;*/
   	 
   	 
   	//tot_gc_count += data[i].gcCount;
   	tot_ops += data[i].ops;
   	//totinswin += data[i].tot_ins_wt;
   	//totdelwin += data[i].tot_del_wt;
	/*tot_rec_dealloc+= data[i].recDealloc;
   	tot_ptr_dealloc+= data[i].ptrDealloc;
   	tot_data_dealloc+= data[i].dataDealloc;
   	tot_ptr_free+= data[i].freePtr;
   	tot_data_free+= data[i].freeData;
   	tot_rec_free+= data[i].freeRec;

	tnpc+= data[i].npc;
	tndc+= data[i].ndc;
	tnrc+= data[i].nrc;

	trpc+= data[i].rpc;
	trdc+= data[i].rdc;
	trrc+= data[i].rrc;*/
	
	
	//inscases += data[i].inscase1;
	///inswin += data[i].inswinop;
	//retries+= data[i].maxretry;
	
	//tot_helped+= data[i].helped;
	
	//fdel+= data[i].fdelCas;
   }
   
    tot_inserts+= pre_inserts;
     unsigned long tot_updates = 0;
 for(int i = reader_threads; i < nb_threads; i++){
 	tot_updates += data[i].ops;
 }
    std::cout << "Total Number of Nodes ( " << tot_inserts << ", " << tot_deletes << " ) = " << tot_inserts - tot_deletes << std::endl;
   std::cout << "Total Number of leaf nodes (sanity check) = " << leafNodes << std::endl;
   
   if(leafNodes != (tot_inserts-tot_deletes)){
   	std::cout << "Error. mismatch!!" << std::endl;
   	exit(0);
   }
   
  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
  
  std::cout << "*--PERFORMANCE STATISTICS--------*" << std::endl;
  std::cout << "Total Operations per sec = " << (tot_ops * 1000)/duration << std::endl; 
 /* std::cout << "Total reads = " << tot_reads << std::endl;
  std::cout << "Total updates = " << (tot_ops - tot_reads) << std::endl;
  std::cout << "AVG read time(usec) = " << tot_read_time/tot_reads << std::endl; 
  std::cout << "*--------------------------------------*" << std::endl;
  std::cout << "Number of Insert opns = " << tot_insert_count<< std::endl;
  std::cout << "Avg. Slow Insert Time(usec) = " << tot_insert_time/tot_insert_count<< std::endl;

  std::cout << "\n";
  std::cout << "Number of Update opns = " << tot_update_count<< std::endl;
  std::cout << "Avg. Fast Insert Time(usec) = " << tot_update_time/tot_update_count<< std::endl;
  std::cout << "\n";
  std::cout << "Number of Slow Delete opns = " << tot_slowdel_count<< std::endl;
  std::cout << "Avg. Slow Delete Time(usec) = " << tot_slowdel_time/tot_slowdel_count<< std::endl;
  std::cout << "\n";

  std::cout << "Number of Fast Delete opns = " << tot_fastdel_count<< std::endl;
  std::cout << "Avg. Fast Delete Time(usec) = " << tot_fastdel_time/tot_fastdel_count<< std::endl;
  std::cout << "\n";
  
  std::cout << "Number of Modify Operations = " << (tot_fastdel_count+tot_slowdel_count+tot_insert_count+tot_update_count)<< std::endl;
  std::cout << "Avg. Modify Operation Time(usec) = " << (tot_fastdel_time+tot_slowdel_time+tot_insert_time+tot_update_time)/(tot_fastdel_count+tot_slowdel_count+tot_insert_count+tot_update_count)<< std::endl;*/
  
  //std::cout << "Average Number of Insert Window Transactions Executed = " << (double)totinswin/tot_insert_count << std::endl;
  //std::cout << "Average Number of Delete Window Transactions Executed = " << (double)totdelwin/tot_slowdel_count << std::endl;
  
  /*std::cout << "*--GARBAGE COLLECTION STATISTICS--------*" << std::endl;
  std::cout << "AVG gc time(sec) = " << tot_gc_time/tot_gc_count << std::endl; 	
  std::cout << "TOTAL gc time(sec) = " << tot_gc_time << std::endl; 	
  std::cout << "TOTAL gc Instances = " << tot_gc_count << std::endl;
  std::cout << "*--------------------------------------*" << std::endl;
  std::cout << "Total PtrNodes Deallocated =" << tot_ptr_dealloc << std::endl; 
  std::cout << "Total DataNodes Deallocated = " << tot_data_dealloc << std::endl;
  std::cout << "Total Records Deallocated =" << tot_rec_dealloc << std::endl; 
  std::cout << "Total PtrNodes freed =" << tot_ptr_free << std::endl; 
  std::cout << "Total DataNodes freed = " << tot_data_free << std::endl; 
  std::cout << "Total Records freed = " << tot_rec_free << std::endl; 
  std::cout << "*--------------------------------------*" << std::endl;
  std::cout << "Total PtrNodes Allocated =" << tnpc << std::endl; 
  std::cout << "Total DataNodes Allocated = " << tndc << std::endl;
  std::cout << "Total Records Allocated =" << tnrc << std::endl; 
  std::cout << "Total PtrNodes Reused =" << trpc << std::endl; 
  std::cout << "Total DataNodes Reused = " << trdc << std::endl; 
  std::cout << "Total Records Reused = " << trrc << std::endl;  */
  

  
  free(threads);
  delete [] data;
  return 0;
}
