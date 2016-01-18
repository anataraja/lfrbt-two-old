//#include <linux/atomic.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "wfrbt.h"




int main(){

node_t * newNode = (node_t *)malloc(sizeof(node_t));

//node_t * newNode1 = (node_t *)malloc(sizeof(node_t));

//std::cout << "newNode1__" << newNode1 << std::endl;

newNode->lChild =3194798272 ;
 //create_child_word(newNode1,INTERNAL,ONEED);

std::cout << "child__" << (node_t *)get_child(newNode->lChild) << std::endl;
//std::cout << "is_leaf__" << is_ext(newNode->lChild) << std::endl;
std::cout << "status__" << get_child_status(newNode->lChild) << std::endl;

AO_t word =124660536341  ;// combine_oprec_status_child_opdata(NULL,ANNOUNCE,RIGHT); //4503567416215574;
std::cout << "oprec__" <<  (oprec_t *)extract_oprec_from_opdata(word) << std::endl;
std::cout << "status__" << extract_status_from_opdata(word) << std::endl;
std::cout << "child__" << extract_child_from_opdata(word) << std::endl;
std::cout << "marked__" << is_node_marked(word) << std::endl;
std::cout << "pos__" << (node_t *)extract_position_from_oprecord(281474077953408) << std::endl;

}
