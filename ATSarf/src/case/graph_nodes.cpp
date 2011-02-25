#include "graph_nodes.h"
#include "narrator_abstraction.h"

#define min(a,b) a>b?b:a

NULLChainNarratorNode nullChainNarratorNode;
NULLNarratorNodeIfc nullNarratorNodeIfc;
NULLGraphNarratorNode nullGraphNarratorNode;
ChainsContainer chains;

NarratorNodeIfc & ChainNarratorNode::getCorrespondingNarratorNode()
{
	if (graphNode!=NULL)
		return (*graphNode);
	else
		return (*this);
}

ChainNarratorNode & ChainNarratorNode::prevInChain()
{
	return (previous!=NULL
				?(*previous)
				:nullChainNarratorNode);
}

ChainNarratorNode & ChainNarratorNode::nextInChain()
{
	return (next!=NULL
				?(*next)
				:nullChainNarratorNode);
}
