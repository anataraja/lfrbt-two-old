/* ################################################################### *
	HELPING FUNCTIONS
 * ################################################################### */
 
void * help_within_window(thread_data_t * data, oprec_t * O, node_t * current, AO_t curContents, int sucid){
 	
 	//dataNode_t * dnode = (dataNode_t *)extract_dnode_from_ptrnode(curContents);
 	Word posWord = O->windowLoc;
 	
 	if(extract_status_from_opdata(curContents) == DONE){
 		// Operation has terminated. Update O->windowLoc
 		//if((node_t *)extract_position_from_oprecord(posWord) == current){
			update_oprecord(data, O, current, curContents,13);
 			
 		return NULL;
 	}
 	else if(extract_status_from_opdata(curContents) == ANNOUNCE){
 		
 		if(lock_invariant_path(data, O)){
 			upgrade_announcement(data, O);
 		}
 		else{
 			release_announcement(data, O);
 			
 			return NULL;
 		}
 		
 	}
 	else if(extract_status_from_opdata(curContents) == PLOCK){
 		node_t * lastLoc = (node_t *)extract_position_from_oprecord(posWord);
 		
 		if(lastLoc != current){
 			// O has already been updated
 			AO_t pNodeCheck = (current->opData);
 			if(pNodeCheck != curContents){
 				// operation has moved down. Node is free.
 				data->lastCase = 3723;
				return NULL;
 			}
 			else{
 				AO_t lastContents = (lastLoc->opData);
 				//dataNode_t * lastDNode = (dataNode_t *)extract_dnode_from_ptrnode(lastContents);
 				
 				data->lastCase = 3744;
 				if((extract_status_from_opdata(lastContents) == DONE) && ((oprec_t *)extract_oprec_from_opdata(lastContents) == O)){
 					help_within_window(data, O, lastLoc, lastContents,sucid);
 					
 				}
 			}
 		}
		
		// check that operation is still pending
		int child = extract_child_from_opdata(curContents);
		AO_t childWord;
		if(child == LEFT){
			childWord = current->lChild;
		}
		else{
			childWord = current->rChild;
		}
		
		if(get_child_status(childWord) == ONEED){
			// operation has been completed. Change node status to DONE
			int result = atomic_cas_full1(data, current, curContents, combine_oprec_status_child_opdata(O,DONE,child));
			if(result == 1){
			if((node_t *)get_child(childWord) == O->sr->addresses[1]){
				std::cout << "same child, but status done" << std::endl;
			}
				//std::cout << "CASFAIL__100" << std::endl;
				//exit(0);
			}
			
			curContents = current->opData;
			if((extract_status_from_opdata(curContents) == DONE) && ((oprec_t *)extract_oprec_from_opdata(curContents) == O)){
 					help_within_window(data, O, current, curContents,sucid);

 				data->lastCase = 3777;
 					return NULL;
 				}
		}
		
 		
 	}
 	
 	perform_one_window_operation(data, current, O, 553);
 				//data->lastCase = 3786;
 	
}

int help_during_validation(thread_data_t * data, oprec_t * O, node_t * pNode, AO_t pNodeContents,int sucid){

	if((pNode->opData) != pNodeContents){
		return 65;
	}

	if(extract_status_from_opdata(pNodeContents) == ANNOUNCE){
		if(lock_invariant_path(data, O)){
			upgrade_announcement(data, O);
			
			perform_one_window_operation(data, pNode, O, 563);
			return 73;
		}
		else{
			release_announcement(data, O);
			return 77;
		}
	}
	
	else if(extract_status_from_opdata(pNodeContents) == PLOCK){
		AO_t posWord = O->windowLoc;
		node_t * lastLoc = (node_t *)extract_position_from_oprecord(posWord);
		
		if(lastLoc != pNode){
			AO_t pNodeCheck = (pNode->opData);
 			if(pNodeCheck != pNodeContents){
 				// operation has moved down.
 				return 89;
 			}
 			else{
 				AO_t lastContents = (lastLoc->opData);
 				//dataNode_t * lastDNode = (dataNode_t *)extract_dnode_from_ptrnode(lastContents);
 				
 				if((extract_status_from_opdata(lastContents) == DONE) && ((oprec_t *)extract_oprec_from_opdata(lastContents) == O)){
					// same operation. need to update oprec.
					help_within_window(data, O, lastLoc, lastContents,sucid);
					return 101;
 				}
 				return 102;
 			}
 			
		}
		
		int temp = perform_one_window_operation(data, pNode, O, 592);
		return temp;
		
		
		
	}
	
	else{
		Word posWord = O->windowLoc;
		node_t * curLoc = (node_t *)extract_position_from_oprecord(posWord);
		
		if(curLoc == NULL){
			// operation has already terminated.
			//data->lastCase = 123;
			return 124;
		}
		
		AO_t curContents = (curLoc->opData);
		//dataNode_t * curDNode = (dataNode_t *)extract_dnode_from_ptrnode(curContents);
		
		if((oprec_t *)extract_oprec_from_opdata(curContents) == O){
			if((extract_status_from_opdata(curContents) == PLOCK) || (extract_status_from_opdata(curContents) == ANNOUNCE)){
				//data->lastCase = 789;
				return(help_during_validation(data, O, curLoc, curContents,sucid));
				
			}
			else if((extract_status_from_opdata(curContents) == DONE)){
				//help_within_window(data, O, curLoc, curContents,sucid);
				update_oprecord(data, O, curLoc, curContents,154);
	 			return 141;
			}
			else{
				//data->lastCase = 456;
				return 145;
			}
		}
		
	}
	//data->lastCase = 789;
	return 151;
	//return NULL;
	
}

