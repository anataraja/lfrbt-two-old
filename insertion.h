#include <iostream>
#include "wfrbt.h"



bool is_external_node(node_t * current){
	
	
	
	if(((node_t *)get_child(current->lChild) == NULL) && ((node_t *)get_child(current->rChild) == NULL)){
		return true;
	}	
	
	return false;
	

}

bool is_parent_of_external_node(node_t * current, long key){
	node_t * next = NULL;
	node_t * sib = NULL;
//	if(key <= current->key){
		next = (node_t *)get_child(current->lChild);
		sib = (node_t *)get_child(current->rChild);
//	}
//	else{
//		next = (node_t *)get_child(current->rChild);
//	}
	
	if(next == NULL || sib == NULL){
		std::cout << "Error. Leaf node, not parent." << std::endl;
		exit(0);
	}
	if((is_external_node(next)) || (is_external_node(sib))){
		return true;
	}	
	
	return false;
	

}


node_t * check_insert_case_2(thread_data_t * data, node_t * wRootChildCopy, long key, int currentWindowSize, oprec_t * O ){
	
		//std::cout << "cas2" << std::endl;	
	// check if a black node with a black child is encountered
	
	
	node_t * nextNode;
	if(key <= wRootChildCopy->key){
		nextNode = (node_t *)get_child(wRootChildCopy->lChild);
	}
	else{
		nextNode = (node_t *)get_child(wRootChildCopy->rChild);
	}
	//dataNode_t * nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
	if(is_external_node(nextNode)){
		// external node encountered... return

		return(NULL);
	}
	#ifdef DEBUG
	if(nextNode->seq != O->seq){
		std::cout << "Diff_seq_58" << std::endl;
	}
	#endif
	int depth = 1;
	while(depth < (WINDOW_SIZE - 1)){
		//nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
	#ifdef DEBUG
		if(nextNode->seq != O->seq){
			std::cout << "Diff_seq_70" << std::endl;
		}
	#endif
		if(nextNode->color == RED){
		
			// find next node on access path
			if(key <= nextNode->key){
				nextNode = (node_t *)get_child(nextNode->lChild);
			}
			else{
				nextNode = (node_t *)get_child(nextNode->rChild);
			}

			if(is_external_node(nextNode)){
				// external node encountered... return
				return(NULL);
			}
			
			depth++;
			if(depth >= (currentWindowSize - 1))
				return NULL;
			else
				continue;	
		}
		
		else{
			if(is_external_node(nextNode)){
				// external node encountered... return
				return(NULL);
			}
			node_t * leftChild = (node_t *)get_child(nextNode->lChild);
			node_t * rightChild = (node_t *)get_child(nextNode->rChild);
			
			if(leftChild != NULL){
				//dataNode_t * leftChildDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(leftChild));
					#ifdef DEBUG
						if(leftChild->seq != O->seq && !is_external_node(leftChild)){
							std::cout << "Diff_seq_107" << std::endl;
						}
					#endif
				if(leftChild->color == BLACK){
					if(!is_external_node(leftChild)){
						leftChild->opData = combine_oprec_status_child_opdata(O,SLOCK,LEFT);
						//leftChild->slocker = 100;
					}	
					//*(leftChild) = combine_dnode_status_ptrnode(leftChildDNode,SLOCK);
					//if(nextNode->opData != NULL){
				//		std::cout << "109" << std::endl;
				//	}
					nextNode->opData = combine_oprec_status_child_opdata(O,SLOCK,LEFT);
				//	nextNode->slocker = 104;
					return nextNode; // one child is black of black node
				}	
			}
			if(rightChild != NULL){
				//dataNode_t * rightChildDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(rightChild));
				#ifdef DEBUG
				if(rightChild->seq != O->seq && !is_external_node(rightChild)){
					std::cout << "Diff_Seq_124" << std::endl;
				}	
				#endif
				if(rightChild->color == BLACK){
				if(!is_external_node(rightChild)){
					//if(rightChild->opData != NULL)
					//	std::cout << "121" << std::endl;
					rightChild->opData = combine_oprec_status_child_opdata(O,SLOCK,LEFT);
					//rightChild->slocker = 113;
				}	
					//if(nextNode->opData != NULL)
					//	std::cout << "126" << std::endl;
					nextNode->opData = combine_oprec_status_child_opdata(O,SLOCK,RIGHT);
					//nextNode->slocker = 116;
					return nextNode; // one child is black of black node
				}	
			}
			// both children are red

			if(key <= nextNode->key){
				nextNode = leftChild;
			}
			else{
				nextNode = rightChild;
			}

			
			
			
			
			depth++;
			if(depth >= (currentWindowSize - 1))
				return NULL;
			else
				continue;	
			
		}
		
	}
	
	return NULL;
} 


node_t * check_insert_case_1(node_t * wRootChildCopy, long key, int currentWindowSize, oprec_t * O){
	
	//std::cout << "cas1" << std::endl;	
	int depth = 1;
	node_t * cur = wRootChildCopy;
	node_t * next = NULL;
	node_t * last = NULL;
	while(depth <= (WINDOW_SIZE - 1)){
			
		if(is_external_node(cur)){
			if(depth > 0){
				return(last);
			}
			else{
				std::cout << "ERROR!! EXTERNAL NODE ENCOUNTERED AT DEPTH 0" << std::endl;
				exit(0);
			}
		}
		#ifdef DEBUG
		if(cur->seq != O->seq){
			std::cout << "Diff_seq_186" << std::endl;
		}	
		#endif		
		if(key <= cur->key){
			next = (node_t *)get_child(cur->lChild);
		}
		else{
			next = (node_t *)get_child(cur->rChild);
		}		
			
		depth++;
		if(depth > (currentWindowSize)){
			
			return NULL; // does not satisfy case 1
		}
		last = cur;
		cur = next;	
	}
	return NULL;
	// satisfies case 1
	
	
}


node_t * balance_after_insertion(thread_data_t * data, node_t * cur, node_t * wRootChildCopy){
	
		

	//dataNode_t *curDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(curPtrNode));

		node_t * pNode = cur->parent;
		//node_t *pDataNode = NULL;
		if(pNode == NULL){
			//std::cout << "cbi1" << std::endl;
				pNode = wRootChildCopy;

                     
		}
		
               // else{
             //   std::cout << "cbi2" << std::endl;
               //     pDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(pPtrNode));
               // }

		//NOTE: Children of windowRoot also have NULL for parent
		

		if(pNode->color == BLACK){
			//std::cout << "cbi174" << std::endl;
			//windowRootDataNode->balcase = 179;
			return wRootChildCopy;
		}

	// now current node and its parent are red

	node_t * pPNode = pNode->parent; // Parent of parent node
	//dataNode_t * pPDataNode = NULL;
	if(pPNode == NULL){
		// Parent of parent does not exist.
        if(pNode == wRootChildCopy){
			//std::cout << "cbi186" << std::endl;
			pNode->color = BLACK;
			//pDataNode->balcase = 192;
		
			return pNode;
        }
	    else{
			// parent is child of windowRoot
			pPNode = wRootChildCopy;
		}

	}
	//else{
	//	pPDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(pPPtrNode));
	//}

	

	node_t * pSNode = NULL;

      
	 if((node_t *)get_child(pPNode->lChild) == pNode){
		pSNode = (node_t *)get_child(pPNode->rChild);
	 }
	else if((node_t *)get_child(pPNode->rChild) == pNode){
		pSNode = (node_t *)get_child(pPNode->lChild);
	}
        else{
               std::cout << "The garbage is here" << std::endl;
               exit(0);
        }


	
		//dataNode_t *pSDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(pSPtrNode));

		// case 3b
		if(pSNode->color == RED){
			//std::cout << "3b" << std::endl;
			// both parent and uncle are red
			// color parent and uncle black, color parent(parent) red and proceed upwards
			
			pSNode->color = BLACK;
			pNode->color = BLACK;
			pPNode->color = RED;
			// Only the root of the tree can have two of its children red
			if(pPNode == wRootChildCopy){
				//windowRootDataNode->balcase = 237;
				return wRootChildCopy;
			}
			// otherwise we proceed upwards
			
			node_t * finalRootNode = balance_after_insertion(data, pPNode, wRootChildCopy);
			
			//oprec_t * O = windowRootDataNode->opData;
			//windowRootDataNode->opData = NULL;
			//finalRootDataNode->balcase = 246;
			return finalRootNode;
		}
		else{
			/// case 3d and 3e - Uncle is black
			//std::cout << "3d,3e" << std::endl;

			node_t * pPPNode = NULL;
			//dataNode_t *pPPDataNode = NULL;
			if(pPNode == wRootChildCopy){
				pPPNode = NULL;
			}
			else{
				pPPNode = pPNode->parent;
				if(pPPNode == NULL){
					pPPNode = (wRootChildCopy);
				}
				//else{
				//	pPPDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(pPPPtrNode));
				//}
			}
		
			// case 3d
			if((node_t *)get_child(pNode->lChild) == cur){

				// curNode is left Child of parent
				// case 3d(a)
				//std::cout << "3d(a)" << std::endl;
				if((node_t *)get_child(pPNode->lChild) == pNode){
					// parent is also left child
					//std::cout << "3d(a)LL" << std::endl;
					if(pPPNode == NULL){

					pPNode->lChild = pNode->rChild;
					
					//node_t * newPPNode = (node_t *)get_new_node(data);
				
					
					//newPPPtrNode->field.AO_val1 = combine_move_and_procid(NULL, 0);
					//newPPPtrNode->field.AO_val2 = combine_dnode_and_flag(pPDataNode,0);
					
					//*(newPPPtrNode) = combine_dnode_status_ptrnode(pPDataNode,FREE);
					
					pNode->rChild = create_child_word(pPNode,OFIN);
					pNode->color = BLACK;
					pPNode->color = RED;
					
					//oprec_t * O = windowRootDataNode->opData;
					//windowRootDataNode->opData = NULL;
					//pDataNode->balcase = 295;
					return pNode;
					}
					else{
					pPNode->lChild = pNode->rChild;
					
					pNode->rChild = create_child_word(pPNode,OFIN);
					pNode->color = BLACK;
					pPNode->color = RED;
						if((node_t *)get_child(pPPNode->lChild) == pPNode){
							pPPNode->lChild = create_child_word(pNode,OFIN);
						}
						else if((node_t *)get_child(pPPNode->rChild) == pPNode){
							pPPNode->rChild = create_child_word(pNode,OFIN);
						}
						else{
							std::cout << "This should never happen. Failure in Case LL " << std::endl;
							exit(0);
						}
					}
				}
				else{ 
					// parent is right child
					// parent of parent is root of tree
					//std::cout << "3d(a)RL" << std::endl;
					if(pPPNode == NULL){
						pNode->lChild = cur->rChild;
						pPNode->rChild = cur->lChild;
						cur->rChild = create_child_word(pNode,OFIN);
						
						//pointerNode_t * newPPPtrNode = (pointerNode_t *)get_new_pointerNode(data);
							
						//*(newPPPtrNode) = combine_dnode_status_ptrnode(pPDataNode,FREE);	
						
						//newPPPtrNode->field.AO_val1 = combine_move_and_procid(NULL, 0);
						//newPPPtrNode->field.AO_val2 = combine_dnode_and_flag(pPDataNode,0);
						
					
						cur->lChild = create_child_word(pPNode,OFIN);
						cur->color = BLACK;
						pPNode->color = RED;
					
						//oprec_t * O = windowRootDataNode->opData;
						//windowRootDataNode->opData = NULL;
					
						//curDataNode->balcase = 340;
						return cur;
					}
					else{
						//std::cout << "3d(a)340" << std::endl;
						pNode->lChild = cur->rChild;
						pPNode->rChild = cur->lChild;
						cur->rChild = create_child_word(pNode,OFIN);
						
						cur->lChild = create_child_word(pPNode,OFIN);
						cur->color = BLACK;
						pPNode->color = RED;
						if((node_t *)get_child(pPPNode->lChild) == pPNode){
							pPPNode->lChild = create_child_word(cur,OFIN);
						}
						else if((node_t *)get_child(pPPNode->rChild) == pPNode){
							pPPNode->rChild = create_child_word(cur,OFIN);
						}
						else{
							std::cout << "This should never happen. Failure in Case LR " << std::endl;
							exit(0);
						}
					}
				}

			}
			else{ // current node is right child of parent
				if((node_t *)get_child(pPNode->lChild) == pNode){
					// case 3e
					//std::cout << "3e(a)" << std::endl;
					if(pPPNode == NULL){
						pNode->rChild = cur->lChild;
						pPNode->lChild = cur->rChild;
						cur->lChild = create_child_word(pNode,OFIN);
						//pointerNode_t * newPPPtrNode = (pointerNode_t *)get_new_pointerNode(data);
						//*(newPPPtrNode) = combine_dnode_status_ptrnode(pPDataNode,FREE);	
						
						//newPPPtrNode->field.AO_val1 = combine_move_and_procid(NULL, 0);
						//newPPPtrNode->field.AO_val2 = combine_dnode_and_flag(pPDataNode,0);	
						
						
						
						cur->rChild = create_child_word(pPNode,OFIN);
						cur->color = BLACK;
						pPNode->color = RED;
						
						//oprec_t * O = windowRootDataNode->opData;
						//windowRootDataNode->opData = NULL;
						
						//curDataNode->balcase = 389;
						return cur;
					}
					else{
						//std::cout << "3e(b)" << std::endl;
						pNode->rChild = cur->lChild;
						pPNode->lChild = cur->rChild;
						cur->lChild = create_child_word(pNode,OFIN);
						
						cur->rChild = create_child_word(pPNode,OFIN);
						cur->color = BLACK;
						pPNode->color = RED;
						if((node_t *)get_child(pPPNode->lChild) == pPNode){
							pPPNode->lChild = create_child_word(cur,OFIN);
						}
						else if((node_t *)get_child(pPPNode->rChild) == pPNode) {
							pPPNode->rChild = create_child_word(cur,OFIN);
						}
						else{
							std::cout << "This should never happen. Failure in Case RL " << std::endl;
							exit(0);
						}
					}
				}
				else if((node_t *)get_child(pPNode->rChild) == pNode)
					{ // parent is right child
					//std::cout << "3eRR" << std::endl;
					if(pPPNode == NULL){
						pPNode->rChild = pNode->lChild;
						//pointerNode_t * newPPPtrNode = (pointerNode_t *)get_new_pointerNode(data);
						//*(newPPPtrNode) = combine_dnode_status_ptrnode(pPDataNode,FREE);	
					
						
						
						//newPPPtrNode->field.AO_val1 = combine_move_and_procid(NULL, 0);
						//newPPPtrNode->field.AO_val2 = combine_dnode_and_flag(pPDataNode,0);	
						pNode->lChild = create_child_word(pPNode,OFIN);
						pNode->color = BLACK;
						pPNode->color = RED;
						
						//oprec_t * O = windowRootDataNode->opData;
						//windowRootDataNode->opData = NULL;
						//pDataNode->balcase = 431;
						
						return pNode;

					}
					else{
						//std::cout << "3e433" << std::endl;
						pPNode->rChild = pNode->lChild;
						
						pNode->lChild = create_child_word(pPNode,OFIN);
						pNode->color = BLACK;
						pPNode->color = RED;

						if((node_t *)get_child(pPPNode->lChild) == pPNode){
							pPPNode->lChild = create_child_word(pNode,OFIN);
						}
						else if((node_t *)get_child(pPPNode->rChild) == pPNode){
							pPPNode->rChild = create_child_word(pNode,OFIN);
						}
						else{
							std::cout << "This should never happen. Failure in Case RR " << std::endl;
							exit(0);
						}
					}
				}
				else{
					std::cout << "Aha! What new garbage is this?" << std::endl;
					exit(0);
				}

			}
		}

	return NULL;

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
node_t * handle_case_3(thread_data_t * data, int pid, node_t * wRoot, node_t * wRootChild, node_t * wRootChildCopy, long key, int currentWindowSize, AO_t casField, oprec_t * O){

	//_t *table = *(data->table);
	//std::cout << "cas3" << std::endl;
	
	node_t * lc = (node_t *)get_child(wRootChildCopy->lChild);
	//dataNode_t * lcd = (dataNode_t *)extract_dnode_from_ptrnode(*(lcp));
	
	node_t * rc = (node_t *)get_child(wRootChildCopy->rChild);
	//dataNode_t * rcd = (dataNode_t *)extract_dnode_from_ptrnode(*(rcp));
	#ifdef DEBUG
	
	if(lc->seq != O->seq || rc->seq != O->seq){
		std::cout << "Diff_seq_543" << std::endl;
	}
	#endif
	int depth = 0;
	node_t * nextNode;
	
	if(key <= wRootChildCopy->key){
		nextNode = lc;
	}
	else{
		nextNode = rc;
	}
	
	//dataNode_t * nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
	if(nextNode->color == BLACK){
		depth = 1; // Y is X as per fig 4 of Tarjan's paper
		while(depth <= 5){//  (WINDOW_SIZE - 1)){
			// set nextDNode color to red	
		
			nextNode->color = RED;
			node_t * leftChild = (node_t *)get_child(nextNode->lChild);
			node_t * rightChild = (node_t *)get_child(nextNode->rChild);
		
		#ifdef DEBUG
			if(leftChild->seq != O->seq && !is_external_node(leftChild)){
				std::cout << "Diff_seq_565" << std::endl;
			}
			if(rightChild->seq != O->seq && !is_external_node(rightChild)){
				std::cout << "Diff_seq_568" << std::endl;
			}
		#endif	
	
			leftChild->color = BLACK;
			rightChild->color = BLACK;
			
			depth+= 2;
			if(depth < WINDOW_SIZE){
			if(key <= nextNode->key){
				if(key <= leftChild->key){
					nextNode = (node_t *)get_child(leftChild->lChild);
				}
				else{
					nextNode = (node_t *)get_child(leftChild->rChild);
				}
			}
			else{
				if(key <= rightChild->key){
					nextNode = (node_t *)get_child(rightChild->lChild);
				}
				else{
					nextNode = (node_t *)get_child(rightChild->rChild);
				}
				
			}
		#ifdef DEBUG
			if(nextNode->seq != O->seq){
				std::cout << "Diff_Seq_597" << std::endl;
			}
		#endif	
			}
			
			//nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
		}
	
		nextNode->color = RED;
		node_t * leftChild1 = (node_t *)get_child(nextNode->lChild);
		node_t * rightChild1 = (node_t *)get_child(nextNode->rChild);
	#ifdef DEBUG
			
		if(leftChild1->seq != O->seq){
			std::cout << "Diff_Seq_610" << std::endl;
		}	
		if(rightChild1->seq != O->seq){
			std::cout << "Diff_Seq_613" << std::endl;	
		}
	#endif		
		leftChild1->color = BLACK;
		rightChild1->color = BLACK;
		
		if(key <= nextNode->key){
			nextNode = leftChild1;
		}
		else{
			nextNode = rightChild1;
		}
		
		//TODO: SET STATUS OF child of nextNode to PLOCK

		
	 	
	 	if(nextNode != NULL){
		 		
		 		//dataNode_t * dh = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));	
		 		
		 		//dh->opData = O; // set opData field for root of next window
				node_t * nextRoot = nextNode->parent;
				if((node_t *)get_child(nextRoot->lChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,LEFT);
				}
				else if((node_t *)get_child(nextRoot->rChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,RIGHT);
				}
				else{
					std::cout << "Parent Pointer incorrectly assigned. 576" << std::endl;
					exit(0);
				}
				
				wRootChildCopy->move = nextRoot;
				//wRootChildCopy->mover = 616;
				nextNode->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);	
		 		//nextNode->slocker = 616;
		 			
		 		node_t * lc = (node_t *)get_child(nextNode->lChild);
		 		
		 		if(!is_external_node(lc)){
		 			lc->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);
		 			//lc->slocker = 622;
		 		}
				
				//pointerNode_t * rc = dh->rChild;
		 		
		 		//dataNode_t * ldh = (dataNode_t *)extract_dnode_from_ptrnode(*(lc));
		 		//ldh->opData = O;
		 		//dataNode_t * rdh = (dataNode_t *)extract_dnode_from_ptrNode_field2(rc->field.AO_val2);
		 		
		 		//*(lc) = combine_dnode_status_ptrnode(ldh,SLOCK);
		 		
		 	}
		else{
			wRootChildCopy->move = NULL;
		}
		//rootDNode->op = -1;
		//rootDNode->rootCount = 584;
		//AO_double_t localRootPtr = (casField);
		//AO_t newRootPtrNode;
		
		
						
		//	newRootPtrNode = combine_dnode_flag_ptrnode(rootDNode,FREE);
		//newRootPtrNode.AO_val2 = combine_dnode_and_flag(rootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
		//check_window(data, wRootChildCopy, currentWindowSize, key);
		
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		
		//int result = atomic_cas_full1(data,wRootPNode, casField, combine_dnode_status_ptrnode(rootDNode,DONE));
			if(result1 == 1){
									
				// cas success
				//update_rtable_entries(data, wRootPNode, map_datanode_address_to_hplist_entry(rootDNode));
				//flush_window_on_dcas_success(data,pid);
				return NULL;
		
			} 
			else{
				// cas failure. Some other node executed window
				//free_window_on_dcas_failure(data,pid);
				return NULL;
			}			
		
		
	
	}
	
	else{
		// Y is the child of X
		node_t * Y = nextNode;
		
		if(key <= nextNode->key){
			nextNode = (node_t *)get_child(nextNode->lChild);
		}
		else{
			nextNode = (node_t *)get_child(nextNode->rChild);
		}
		node_t * Y1 = nextNode;
		depth = 2;
		
		//nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
		
	
		
		while(depth <= 6){
			// set nextDNode color to red	
			nextNode->color = RED;
			node_t * leftC = (node_t *)get_child(nextNode->lChild);
			node_t * rightC = (node_t *)get_child(nextNode->rChild);
			
			leftC->color = BLACK;
			rightC->color = BLACK;
			depth+= 2;
			if(depth < WINDOW_SIZE){
			if(key <= nextNode->key){
				if(key <= leftC->key){
					nextNode = (node_t *)get_child(leftC->lChild);
				}
				else{
					nextNode = (node_t *)get_child(leftC->rChild);
				}
			}
			else{
				if(key <= rightC->key){
					nextNode = (node_t *)get_child(rightC->lChild);
				}
				else{
					nextNode = (node_t *)get_child(rightC->rChild);
				}
				
			}
			//nextNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
			}
				
			
		}
		
		
		nextNode->color = RED;
		node_t * leftC1 = (node_t *)get_child(nextNode->lChild);
		node_t * rightC1 = (node_t *)get_child(nextNode->rChild);
		
			leftC1->color = BLACK;
			rightC1->color = BLACK;
			
		if(key <= nextNode->key){
			nextNode = leftC1;
		}
		else{
			nextNode = rightC1;
		}	
		
		// TODO: Set status of child of nextNode to SLOCK and opData to O
		
		node_t * finalWRoot = balance_after_insertion(data, Y1, wRootChildCopy);
		
		
		//if(finalWindowRootDataNode != rootDNode){
			//rootDNode->op = operation;
			
		//}
		
		if(finalWRoot != NULL){
	 	
	 		if(nextNode != NULL){
		 		//nextNode->field.AO_val1 = combine_move_and_procid(NULL,pid);
				
				node_t * nextRoot = nextNode->parent;
				if((node_t *)get_child(nextRoot->lChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,LEFT);
				}
				else if((node_t *)get_child(nextRoot->rChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,RIGHT);
				}
				else{
					std::cout << "Parent Pointer incorrectly assigned. 728" << std::endl;
					exit(0);
				}
				
				finalWRoot->move = nextRoot;
				//finalWRoot->mover = 774;
				nextNode->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);	
		 		//nextNode->slocker = 774;	
		 		node_t * lc = (node_t *)get_child(nextNode->lChild);
		 		if(!is_external_node(lc)){
		 			lc->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);
		 		//	lc->slocker = 778;
		 		}
				
		 		
		 		
		 	}
		 	else{
		 		finalWRoot->move = NULL;
		 	}
			//finalWindowRootDataNode->op = -1;
			//finalWindowRootDataNode->rootCount = 721;
		 	//AO_double_t localRootPtr = (casField);
		 	//AO_t newRootPtrNode;
		 	
		 	//wRootChildCopy->opData = NULL;
		 	//finalWRootDataNode->opData = O;	
		 	
						
			//newRootPtrNode = combine_dnode_flag_ptrnode(finalWindowRootDataNode,FREE);
		//newRootPtrNode.AO_val2 = combine_dnode_and_flag(rootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
		//check_window(data, finalWRoot, currentWindowSize, key);
		
		//int result = atomic_cas_full1(data,wRootPNode, casField, combine_dnode_status_ptrnode(finalWindowRootDataNode,DONE));
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(finalWRoot,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(finalWRoot,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		
		
//		 		newRootPtrNode.AO_val1 = combine_move_and_procid(nextNode, pid);
//		newRootPtrNode.AO_val2 = combine_dnode_and_flag(finalWindowRootDataNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
//		int result = AO_compare_double_and_swap_double1(&wRootPNode->field, localRootPtr.AO_val1, localRootPtr.AO_val2, newRootPtrNode.AO_val1,	newRootPtrNode.AO_val2);
			if(result0 == 1){
									
				// cas success
			//	update_rtable_entries(data, wRootPNode, map_datanode_address_to_hplist_entry(finalWindowRootDataNode));
				//flush_window_on_dcas_success(data,pid);
				return NULL;
		
			} 
			else{
				
				// cas failure. Some other node executed window
				//free_window_on_dcas_failure(data,pid);
				return NULL;
			}				
			
		}
		else{
			
	 		if(nextNode != NULL){
		 		//nextNode->field.AO_val1 = combine_move_and_procid(NULL,pid);
		 		
				node_t * nextRoot = nextNode->parent;
				if((node_t *)get_child(nextRoot->lChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,LEFT);
				}
				else if((node_t *)get_child(nextRoot->rChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,RIGHT);
				}
				else{
					std::cout << "Parent Pointer incorrectly assigned. 806" << std::endl;
					exit(0);
				}
				
				wRootChildCopy->move = nextRoot;
				//wRootChildCopy->mover = 854;
				nextNode->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);	
		 		//	nextNode->slocker = 854;
		 		node_t * lc = (node_t *)get_child(nextNode->lChild);
		 		if(!is_external_node(lc)){
		 			lc->opData = 	combine_oprec_status_child_opdata(O,SLOCK,LEFT);
		 		//	lc->slocker = 858;
		 		}
				
				
		 		
		 	}
		 	else{
		 		wRootChildCopy->move = NULL;
		 	}	
			//rootDNode->op = -1;
			//rootDNode->rootCount = 778;
			
		 	//AO_double_t localRootPtr = (casField);
		 	//AO_t newRootPtrNode;
		 		
		 		
						
			//newRootPtrNode = combine_dnode_flag_ptrnode(rootDNode,FREE);
		//newRootPtrNode.AO_val2 = combine_dnode_and_flag(rootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
		//check_window(data, wRootChildCopy, currentWindowSize, key);
		
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		
		//int result = atomic_cas_full1(data,wRootPNode, casField, combine_dnode_status_ptrnode(rootDNode,DONE));
		 		
		 	
	//	 		newRootPtrNode.AO_val1 = combine_move_and_procid(nextNode, pid);
	//	newRootPtrNode.AO_val2 = combine_dnode_and_flag(rootDNode, extract_flag_from_ptrNode_field2(localRootPtr.AO_val2));
		
	//	int result = AO_compare_double_and_swap_double1(&wRootPNode->field, localRootPtr.AO_val1, localRootPtr.AO_val2, newRootPtrNode.AO_val1,	newRootPtrNode.AO_val2);
			if(result1 == 1){
									
				// cas success
			//	update_rtable_entries(data, wRootPNode, map_datanode_address_to_hplist_entry(rootDNode));
			//	flush_window_on_dcas_success(data,pid);
				return NULL;
		
			} 
			else{
				// cas failure. Some other node executed window
			//	free_window_on_dcas_failure(data,pid);
				return NULL;
			}				
			
			
		}
		
	}
	
}



node_t * get_next_node_on_access_path(thread_data_t * data, int pid, node_t * wRoot, node_t * wRootChild, node_t * wRootChildCopy, long key, int currentWindowSize, AO_t casField, bool case2flag, oprec_t * O){
	

	// CHECK THAT ROOT OF WINDOW IS BLACK
	//Word opn = wRootDNode->op;
		//mTable_t *MT = *(data->table);
	
	node_t * nextNode;
	if(!case2flag ){
		
	nextNode = check_insert_case_2(data, wRootChildCopy, key, currentWindowSize, O);
	if(nextNode == NULL){
	
		//std::cout << "case 2 fail" << std::endl;
	 }
	 else{
	 	//std::cout << "case 2 succeed" << std::endl;
	 	
	 	
	 	//if(nextNode != NULL){
		 		
		 		
		 		//dataNode_t * checkDataNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
		 		//checkDataNode->opData = O;
		 		//*(nextNode) = combine_dnode_status_ptrnode(checkDataNode,PLOCK);
		 		node_t * nextRoot = nextNode->parent;
		 		
		 		if(nextRoot == NULL){
		 			nextRoot = wRootChildCopy;
		 		}
		 		
		 		
				if((node_t *)get_child(nextRoot->lChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,LEFT);
				}
				else if((node_t *)get_child(nextRoot->rChild) == nextNode){
					nextRoot->opData = combine_oprec_status_child_opdata(O,PLOCK,RIGHT);
				}
				else{
					std::cout << "Parent Pointer incorrectly assigned. Ins.h 857" << std::endl;
					exit(0);
				}
	 	//wRootChildCopy->moverop = O;	
	 	wRootChildCopy->move = nextRoot;
	 		
		#ifdef DEBUG
		if(nextRoot == wRootChildCopy){
				wRootChildCopy->mover = 967;		
			}
			else{
				wRootChildCopy->mover = 968;
			}			
		#endif
//check_window(data, wRootChildCopy, currentWindowSize, key);
		
		int child = extract_child_from_opdata(casField);
		int result0;
		if(child == LEFT){
			result0 = atomic_cas_full(&wRoot->lChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		else{
			result0 = atomic_cas_full(&wRoot->rChild, create_child_word(wRootChild,OFIN),create_child_word(wRootChildCopy,ONEED));
		}
		
		int result1 = atomic_cas_full1(data, wRoot, casField, combine_oprec_status_child_opdata(O, DONE, child));
		if(result1 == 1){
			//	wRoot->cseq = 997;
		//	exit(0);
			AO_t childWord;
			if(child == LEFT){
				childWord = wRoot->lChild;
			}
			else{
				childWord = wRoot->rChild;
			}	
		#ifdef DEBUG

			if((node_t *)get_child(childWord) == O->sr->addresses[1]){
				std::cout << "Error--1008" << std::endl;
			}
		#endif	
		}						
		data->madeDecision = true;						
		/*if(result == 1){
		
					
		if(wRootPNode == data->rootOfTree){
			flush_window_on_dcas_success(data,pid);
		}	
		else{
			if(pid == data->id){
				flush_window_on_dcas_success(data,pid);
			}
			else{
				
				flush_window_on_dcas_success(data,pid);
			}
		}	
			return NULL;
		
		} 
		else{
			
			
			free_window_on_dcas_failure(data,pid);
			return NULL;
		} */
		return NULL;
	 }
	 
	 }
	
	// check for case 1		
	// i.e. whether an external node is encountered
	nextNode = check_insert_case_1(wRootChildCopy, key, currentWindowSize,O);
	
	if(nextNode != NULL){
		//std::cout << "case1 succ" << std::endl;
		data->madeDecision = true;
		return (nextNode);
	}
//	Does not satisfy cases 1 and 2. May need to increase the size of the window

	if(currentWindowSize < 10 ){
		return NULL;
	}
	
	//satisfies case 3 - because every black node has 2 red children and an external node was not encountered on the acces path
	 
	 // extend the current window by 1 if Y is not X
	 
	if(key <= wRootChildCopy->key){
		nextNode = (node_t *)get_child(wRootChildCopy->lChild);
	}
	else{
		nextNode = (node_t *)get_child(wRootChildCopy->rChild);
	}
	
	//dataNode_t * nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(*(nextNode));
	
	if(nextNode->color == RED){
	 
		 node_t * lastPtrNode = set_last_node(data, wRootChildCopy, currentWindowSize, key);
		 
		 lastPtrNode = extend_current_window(data, lastPtrNode, key, currentWindowSize, wRoot, casField,O);
		
		node_t * bot = (node_t *)map_word_to_bot_address(1);
		 if(lastPtrNode == bot){
		 	data->madeDecision = true;
			return NULL;	 
		 }	
	 }		
	nextNode = handle_case_3(data, pid, wRoot, wRootChild, wRootChildCopy, key, WINDOW_SIZE - 1, casField, O);
	//std::cout << "****HANDLED CASE 3****" << std::endl;
	// update position in oprecord
	
		
	
	data->madeDecision = true;

	return NULL;
}

