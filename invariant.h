/* ################################################################### *
 * DEPTH AT WHICH INVARIANT HOLDS
 * ################################################################### */
 
 bool check_ins_inv(thread_data_t * data, node_t * pcurrent, AO_t pcurContents, seekrec_t * R, long key){
 	
 		//if(is_parent_of_external_node(pcurrent, key)){
 			// invariant does not hold at parent of leaf node.
 		//	return false;
 		//}
 		
 		node_t * current;
		int isLeft = RIGHT;
		
		if(key <= pcurrent->key){
			current = (node_t *)get_child(pcurrent->lChild);
			isLeft = LEFT;
		}
		else{
			current = (node_t *)get_child(pcurrent->rChild);
		}
		
		if(current == NULL){
			// pcurrent is a leaf node. Invariant is trivially not satisfied.
			return false;
		}
		
		//if(is_parent_of_external_node(current, key)){
 			// invariant does not hold at parent of leaf node.
 		//	return false;
 		//}
		
		AO_t curContents = current->opData;
		
 		node_t * next;
 		node_t * sibling;
 		if(key <= current->key){
 			next = (node_t *)get_child(current->lChild);
 			sibling = (node_t *)get_child(current->rChild);
 		}
 		else{
 			next = (node_t *)get_child(current->rChild);
 			sibling = (node_t *)get_child(current->lChild);
 		}
 		
 		if(next != NULL && sibling != NULL){
 			
 			if(is_external_node(next) || is_external_node(sibling)){
				return false;
			}	
 		
 			if(current->color == BLACK){
 				AO_t nextContents = (next->opData);
 				//dataNode_t * nextDNode = (dataNode_t *)extract_dnode_from_ptrnode(nextContents);
 				
 				AO_t siblingContents = (sibling->opData);
 				//dataNode_t * siblingDNode = (dataNode_t *)extract_dnode_from_ptrnode(siblingContents);
 				
 				if(next->color == BLACK){
 					// invariant satisfied
					R->addresses[0] = pcurrent;
 					R->contents[0] = pcurContents;
					
 					R->addresses[1] = current;
 					R->contents[1] = curContents;
 					
 					R->addresses[2] = next;
 					R->contents[2] = nextContents;
					
					R->isLeft = isLeft;
					
 					return true;
 					 
 				}
 				else if(sibling->color == BLACK){
 					// invariant satisfied
 					R->addresses[0] = pcurrent;
 					R->contents[0] = pcurContents;
					
 					R->addresses[1] = current;
 					R->contents[1] = curContents;
 					
 					R->addresses[2] = sibling;
 					R->contents[2] = siblingContents;
					
					R->isLeft = isLeft;
					
 					return true;
 					
 					
 				}
 				else{
 					// both children of current are red. Invariant is not satisfied.
 					return false;
 				}
 			}	
 		}
 		else{
 			// leaf node. Does not satisfy invariant
 			return false;
 		}
 	
 	
 }
 
bool check_del_inv(thread_data_t * data, node_t * pcurrent, AO_t pcurContents, seekrec_t * R, long key){
 	
 	
 	//dataNode_t * curDNode = (dataNode_t *)extract_dnode_from_ptrnode(curContents);
 	
	bool isLeft = RIGHT;
	
	node_t * current;
	if(key <= pcurrent->key){
		current = (node_t *)get_child(pcurrent->lChild);
		isLeft = LEFT;
	}
	else{
		current = (node_t *)get_child(pcurrent->rChild);
	}
	
	if(current == NULL){
		return false;
	}
	
	AO_t curContents = current->opData;
	
	node_t * child;
 	node_t * lgc; // left grandchild
 	node_t * rgc; // right grandchild
 	
 	AO_t childContents;
 	AO_t lgcContents;
 	AO_t rgcContents;

	
	
 	
 		
 		
 	
 			
 			
 		
 		if(current->color == RED){
 			R->addresses[0] = pcurrent;
 			R->contents[0] = pcurContents;
 			
			R->addresses[1] = current;
 			R->contents[1] = curContents;
			
 			for(int i = 2; i <=3; i++){
 				R->addresses[i] = NULL;
 				R->contents[i] = 0;
 			}
			
			R->isLeft = isLeft;
			
 			return true;
 		}
 		else{
 			// current is not red
 			
 			// First search left subtree of current
 			
 			child = (node_t *)get_child(current->lChild);
 			
 			if(child != NULL){
 				childContents = (child->opData);
 				//dataNode_t * childDNode = (dataNode_t *)extract_dnode_from_ptrnode(childContents);
 				if(child->color == RED){
					R->addresses[0] = pcurrent;
		 			R->contents[0] = pcurContents;
					
 					R->addresses[1] = current;
		 			R->contents[1] = curContents;	
		 			
		 			R->addresses[2] = child;
		 			R->contents[2] = childContents;	
		 			
		 			R->addresses[3] = NULL;
		 			R->contents[3] = 0;	
					
					R->isLeft = isLeft;
					
					return true;		 			
 				}
 				else{
 					if(child->lChild != NULL && child->rChild != NULL){
 						
 						lgc =  (node_t *)get_child(child->lChild);					
 						AO_t lgcContents = (lgc->opData);
		 				
		 				
		 				rgc =  (node_t *)get_child(child->rChild);
		 				AO_t rgcContents = (rgc->opData);
		 				
		 				if(lgc->color == RED){
		 					// invariant satisfied
		 					R->addresses[0] = pcurrent;
		 					R->contents[0] = pcurContents;
							
							R->addresses[1] = current;
		 					R->contents[1] = curContents;
		 					
		 					R->addresses[2] = child;
		 					R->contents[2] = childContents;
		 					
		 					R->addresses[3] = lgc;
		 					R->contents[3] = lgcContents;
							
							R->isLeft = isLeft;
							
		 					return true;
		 					 
		 				}
		 				else if(rgc->color == RED){
		 					// invariant satisfied
		 					R->addresses[0] = pcurrent;
		 					R->contents[0] = pcurContents;
							
							R->addresses[1] = current;
		 					R->contents[1] = curContents;
		 					
		 					R->addresses[2] = child;
		 					R->contents[2] = childContents;
		 					
		 					R->addresses[3] = rgc;
		 					R->contents[3] = rgcContents;
							
							R->isLeft = isLeft;
		 					return true;
		 					
		 					
		 				}
		 				else{
		 					// both grandchildren of current are black. Invariant is not satisfied.
		 					return false;
		 				}
 						
 					}
 					else{
 						return false;
 					}
 				
 				}
 			
 			}
 			
 			
 			// Next search right subtree of current
 			
 			child = (node_t *)get_child(current->rChild);
 			
 			if(child != NULL){
 				childContents = (child->opData);
 				if(child->color == RED){
 					R->addresses[0] = pcurrent;
		 			R->contents[0] = pcurContents;
							
					R->addresses[1] = current;
		 			R->contents[1] = curContents;	
		 			
		 			R->addresses[2] = child;
		 			R->contents[2] = childContents;	
		 			
		 			R->addresses[3] = NULL;
		 			R->contents[3] = 0;	
					
					R->isLeft = isLeft;
					
					return true;		 			
 				}
 				else{
 					if(child->lChild != NULL && child->rChild != NULL){
 						
 						lgc =  (node_t *)get_child(child->lChild);					
 						Word lgcContents = (lgc->opData);
		 				
		 				
		 				rgc =  (node_t *)get_child(child->rChild);
		 				Word rgcContents = (rgc->opData);
		 				
		 				if(lgc->color == RED){
		 					// invariant satisfied
							R->addresses[0] = pcurrent;
		 					R->contents[0] = pcurContents;
							
							R->addresses[1] = current;
		 					R->contents[1] = curContents;
		 					
		 					R->addresses[2] = child;
		 					R->contents[2] = childContents;
		 					
		 					R->addresses[3] = lgc;
		 					R->contents[3] = lgcContents;
							
							R->isLeft = isLeft;
							
		 					return true;
		 					 
		 				}
		 				else if(rgc->color == RED){
		 					// invariant satisfied
		 					R->addresses[0] = pcurrent;
		 					R->contents[0] = pcurContents;
							
							R->addresses[1] = current;
		 					R->contents[1] = curContents;
		 					
		 					R->addresses[2] = child;
		 					R->contents[2] = childContents;
		 					
		 					R->addresses[3] = rgc;
		 					R->contents[3] = rgcContents;
							
							R->isLeft = isLeft;
							
		 					return true;
		 					
		 					
		 				}
		 				else{
		 					// both grandchildren of current are black. Invariant is not satisfied.
		 					return false;
		 				}
 						
 					}
 					else{
 						return false;
 					}
 				
 				}
 			
 			}
 			
 			return false;
		}		
 		
 	
 	
 	
 }

 bool is_invariant_satisfied(thread_data_t * data, node_t * current, AO_t curContents, oprec_t * O, seekrec_t * R){
	
	long key = map_word_to_key(O->op);
	 unsigned opn = map_word_to_operation(O->op); 
	 
	 if(opn == 2){
	 	// insert operation
	 	return(check_ins_inv(data, current, curContents, R, key));
	 	
	 }
	 else{
	 	// delete operation
	 	return(check_del_inv(data, current, curContents, R, key));
	 }
 }
 
 
 
 
bool fastcheck_delete_case_1(thread_data_t * data, long key, node_t * pcurrent, oprec_t * O){
	
	//std::cout << "CHECKING CASE 1" << std::endl;
	node_t * current;
	if(key <= pcurrent->key){
		current = (node_t *)get_child(pcurrent->lChild);
	}
	else{
		current = (node_t *)get_child(pcurrent->rChild);
	}
	
	if(current == NULL){
		// leaf node reached.
		return true;
	}
	
	int depth = 1;
	
	
	
	node_t * next = NULL;
	//AO_t lNode;
	node_t * nextS = NULL;
	//AO_t lSNode;
	node_t * child = NULL;
	//AO_t lchildNode;
	node_t * childS = NULL;
	//AO_t lchildSNode;
	
	node_t * cptr = NULL;
	
	
	
	
	
	
	
	if(key <= current->key){
		next = (node_t *)get_child(current->lChild);
		
		if(next == NULL){
			return true;
		}
		
		//lPtrNode = *(nextPtrNode);
		
		//lPtrNode = *(nextPtrNode);
		
		
		
		//nextDataNode = (dataNode_t *)extract_dnode_from_ptrnode(lPtrNode);
		
		
		nextS = (node_t *)get_child(current->rChild);
		
		if(nextS == NULL){
			return true;
		}
		
		//lSPtrNode = *(nextSPtrNode);
		//lSPtrNode = *(nextSPtrNode);
		
		
		
		//nextSDataNode = (dataNode_t *)extract_dnode_from_ptrnode(lSPtrNode);
	}
	else{
		next = (node_t *)get_child(current->rChild);
		
		if(next == NULL){
			return true;
		}
		
		//lPtrNode = *(nextPtrNode);
		
		//lPtrNode = *(nextPtrNode);
		
		//next = (dataNode_t *)extract_dnode_from_ptrnode(lPtrNode);
		nextS = (node_t *)get_child(current->lChild);
		
		if(nextS == NULL){
			return true;
		}
		
		//lSPtrNode = *(nextSPtrNode);
		//lSPtrNode = *(nextSPtrNode);
		
		
		
		//nextSDataNode = (dataNode_t *)extract_dnode_from_ptrnode(lSPtrNode);		
	}
	
	if(is_external_node(next) || is_external_node(nextS)){
		return true;
	}
	
	while(depth < (DELETE_WINDOW_SIZE - 2)){
		
		if(key <= next->key){
			child = (node_t *)get_child(next->lChild);
			
			//lchildPtrNode = *(childPtrNode);
			
			//lchildPtrNode = *(childPtrNode);
			
				
			
			//childDataNode = (dataNode_t *)extract_dnode_from_ptrnode(lchildPtrNode);
			
			if( is_external_node(child)){
				return true;
			}
			else{
				// child is not external
				next = child;
				
				//nextDataNode = childDataNode;
				
				depth++;
	
			}
		}
		else{
			child = (node_t *)get_child(next->rChild);
			
			//lchildPtrNode = *(childPtrNode);
			
			//lchildPtrNode = *(childPtrNode);
			
			
			
			//childDataNode = (dataNode_t *)extract_dnode_from_ptrnode(lchildPtrNode);
			
			if( is_external_node(child)){
				return true;			
			}
			else{
				// child is not external
				next = child;
				//nextDataNode = childDataNode;
				
				depth++;
	
			}
		}
				
	}

	// does not satisfy case 1
		return false;
	
	
}
