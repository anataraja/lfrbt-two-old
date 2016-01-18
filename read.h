
void seek(thread_data_t * data, oprec_t * O, long key, seekrec_t * R){

	/*if(R->contents[2] != 0){
       				std::cout << "Whats this0?" << std::endl;
       				exit(0);
     	 	}*/
	//std::cout << "SEEK" << std::endl;
	// initially priAddress is tree root, as invariant trivially holds there
	
	node_t * root = data->prootOfTree;
	
	R->addresses[0] =   root;
	R->contents[0] = (R->addresses[0]->opData);
	
	R->addresses[1] = (node_t *)get_child(root->lChild);
	R->contents[1] = R->addresses[1]->opData;
	
	R->isLeft = LEFT;
	
	//dataNode_t * rootDNode = (dataNode_t *)extract_dnode_from_ptrnode(R->contents[0]);
	
	node_t * lastOwnedAddr = NULL;
	AO_t lastOwnedContents = 0;
	
	if(extract_status_from_opdata(R->contents[0]) == PLOCK){
		// root of tree is owned by an operation
		lastOwnedAddr = R->addresses[0];
		lastOwnedContents = R->contents[0];
	}
	
	int opn = map_word_to_operation(O->op);
	
	node_t * current;
	
	node_t * last;
	
	if(key <= root->key){
		current = (node_t *)get_child(root->lChild);
	}
	else{
		current = (node_t *)get_child(root->rChild);
	}
	
	if(current != NULL){
		
	
	
	AO_t curContents = (current->opData);
	
	int depth=0;
	bool flag = false;
	
	while(!is_external_node(current)){
		depth++;
		
		if(opn == 1 && !flag){
			// delete operation.
			//Check if a leaf node is encountered within the window rooted at current, before determining if current or its descendent satisfies invariant
			//std::cout << "Am I here?" << std::endl;
       			//	exit(0);
			if(fastcheck_delete_case_1( data, key,  current, O)){
				flag = true;
			}
			
		
		}
		
		if(!flag){
			/*if(R->contents[2] != 0){
       				std::cout << "Whats this1?" << std::endl;
       				exit(0);
     		  	}*/
			
			if(is_invariant_satisfied(data, current, curContents, O, R)){
		
				// R is already updated
				R->llAddress = lastOwnedAddr;
				R->llContents = lastOwnedContents;
			}
			else{
				if(extract_status_from_opdata(curContents) == PLOCK){
					lastOwnedAddr = current;
					lastOwnedContents = curContents;
				}
			}
			/*if(R->contents[2] != 0){
       				std::cout << "Whats this2?" << std::endl;
       				exit(0);
     		  	}*/
			
		}
		// next node on access path
		node_t * next;
		AO_t nextContents;
		
		if(key <= current->key){
			next = (node_t *)get_child(current->lChild);
		}
		else{
			next = (node_t *)get_child(current->rChild);
		}
		nextContents = (next->opData);
		
		last = current;
		//lastDNode = curDNode;
		
		current = next;
		curContents = nextContents;
		//curDNode = (dataNode_t *)extract_dnode_from_ptrnode(curContents);
		
		if(depth > 1000){
	  		std::cout << "Iterations Exceeded__111r" << std::endl;
	  		exit(0);
  		}
	}
	
	// curDNode is leaf node
	R->leafKey = current->key;
	//return R;
	}
	else{
		R->leafKey = root->key;
	}
	
}


void search(thread_data_t * data, long key){
	
	node_t * root = data->prootOfTree;
	
	while(root != NULL){
		
		if(key <= root->key){
			root = (node_t *)get_child(root->lChild);
		}
		else{
			root = (node_t *)get_child(root->rChild);
		}
	}
	
}

long update_search(thread_data_t * data, long key){
	
	node_t * root = data->prootOfTree;
	
	while(!is_external_node(root)){
		
		if(key <= root->key){
			root = (node_t *)get_child(root->lChild);
		}
		else{
			root = (node_t *)get_child(root->rChild);
		}
	}
	
	return root->key;
	
}




