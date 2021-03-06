#include "Search_by_item.h"
#include "text_handling.h"
#include "diacritics.h"
#include "stem_search.h"
#include <assert.h>

#ifdef USE_TRIE_WALK
bool StemSearch::check_for_terminal(int letter_index,ATTrie::Position pos)
{
	const StemNode * node=NULL;
	Search_StemNode s1;
	if (trie->isTerminal(pos))
	{
		ATTrie::Position pos2=trie->clonePosition(pos);
		trie->walk(pos2, '\0');
		node = trie->getData(pos2);
		trie->freePosition(pos2);
		if (node != NULL)
		{
			id_of_currentmatch=node->stem_id;
			s1.setNode(node);
		}
		else
			return true;
	}
	else
		return true;
	if (!on_match_helper(letter_index,s1))
	{
		stop=true;
		return false;
	}
	return true;
}
#endif

void StemSearch::traverse(int letter_index,ATTrie::Position pos)
{
	int length=info.text->length();
	for (int i=letter_index;i<length && !stop;i++)
	{
#ifdef USE_TRIE
#ifdef USE_TRIE_WALK
		QChar current_letter=info.text->at(i);
		//qDebug()<<"s:"<<current_letter;
		if (isDiacritic(current_letter))
			continue;
		else if (current_letter!=alef)//(!alefs.contains(current_letter))
		{
			if (!trie->walk(pos, current_letter))
				break;
			if (!check_for_terminal(i,pos))
				break;
		}
		else
		{
			for (int j=0;j<alefs.size();j++)
			{
				if (!stop && trie->isWalkable(pos,alefs[j]))
				{
					ATTrie::Position pos2=trie->clonePosition(pos);
					trie->walk(pos2, alefs[j]);
					if (!check_for_terminal(i,pos2))
						break;
					traverse(i+1,pos2);
					trie->freePosition(pos2);
				}
			}
			break;
		}
#else
		QString name=info.text.mid(starting_pos,i-starting_pos);
		const StemNode * node = NULL;
		trie->retreive(name,&node);
		if (node == NULL)
		{
			if (!alefs.contains(name[i]))
			{
				bool matched=false;
				for (int j=0;j<alefs.size();j++)
				{
					trie->retreive(alefs[j] + name.mid(1),&node);
					if (node != NULL){
						matched=true;
						break;
					}
				}
				if (!matched)
					continue;
			}
			else
				continue;
		}
		id_of_currentmatch=node->stem_id;
		Search_StemNode s1(node);
#endif
#else
		Search_by_item s1(STEM,-1);
		bool not_finished=true;
		int j=0;
		QVector<QString> names;
		while (not_finished)
		{
			if (name.size()>0 && alefs.contains(name[0]))
			{
				if  (j<alefs.size())
				{
					QString t=alefs[j]+name.mid(1);
					Search_by_item s2(STEM,t);
					s1=s2;
					j++;
				}
				else
					not_finished=false;
				/*for (int j=0;j<alefs.size();j++)
					names.append(alefs[j]+name.mid(1));
				Search_by_item s2(STEM,name,names);
				s1=s2;
				not_finished=false;*/
			}
			else
			{
				Search_by_item s2(STEM,name);
				s1=s2;
				not_finished=false;
			}
			id_of_currentmatch=s1.ID();
	#endif
#ifndef USE_TRIE_WALK
		if (!on_match_helper(i,s1))
			false_returned=true;
#endif
#ifndef USE_TRIE
		}
#endif
	}
}

bool StemSearch::on_match_helper(int last_letter_index,Search_StemNode & s1)
{
#ifdef REDUCE_THRU_DIACRITICS
#ifndef USE_TRIE_WALK
	currentMatchPos=i-1;
	QString subword=getDiacriticword(i-1,starting_pos,*info.text);
#else
	int last;
	QStringRef subword=addlastDiacritics(starting_pos,last_letter_index,info.text,last);
	currentMatchPos=last>0?last-1:0;
	info.finish=currentMatchPos;
#endif
#if 0
	possible_raw_datas.clear();
	StemNode_info inf;
	category_of_currentmatch=-1;
	while(s1.retrieve(inf))
	{
		if (category_of_currentmatch!=inf.category_id)//based on fact that results of same ctaegory are displayed consecutively
		{
			if (category_of_currentmatch!=-1)
				if (possible_raw_datas.count()>0)
					if (!onMatch())
						return false;
			category_of_currentmatch=inf.category_id;
			possible_raw_datas.clear();
		}
		if (isPrefixStemCompatible())
			if (!reduce_thru_diacritics ||(reduce_thru_diacritics && equal(subword,inf.raw_data)))
				possible_raw_datas.append(inf.raw_data);
	}
	if (category_of_currentmatch!=-1)
		if (possible_raw_datas.count()>0)
			if (!onMatch())
				return false;
#else //I think this is a more efficient implementation, less copying happening in this type of "retrieve"
	while(s1.retrieve(category_of_currentmatch,possible_raw_datas))
	{
		if (isPrefixStemCompatible())
		{
			if (!reduce_thru_diacritics)
			{
				if (!onMatch())
					return false;
			}
			else
			{
				for (int i=0;i<possible_raw_datas.count();i++)
					if (!equal(subword,possible_raw_datas[i]))
					{
						possible_raw_datas.remove(i);
						i--;
					}
				if (possible_raw_datas.count()>0)
					if (!onMatch())
						return false;
			}
		}
	}
#endif

#else
	long cat_id;
	while(s1.retrieve(cat_id))
	{
		if (shouldcall_onmatch(last_letter_index))
		{
			category_of_currentmatch=cat_id;
			currentMatchPos=i-1;
			/*if (info->called_everything)
			{
				if (!onMatch())
					return false;
			}
			else
			{
				if (!info->on_match_helper())
					return false;
			}*/
			if (!onMatch())
				return false;
		}
	}
#endif
	return true;
}

solution_position * StemSearch::computeFirstSolution()
{
	if (solution==NULL)
		solution=new minimal_item_info;
	solution_position * first=new solution_position();

	solution->type=STEM;
	if (multi_p.raw_data)
		solution->raw_data=possible_raw_datas[0];
	else
		solution->raw_data="";
	solution->category_id=category_of_currentmatch;
	if (!multi_p.raw_dataONLY())
	{
		ItemCatRaw2PosDescAbsMapItr itr = database_info.map_stem->find(Map_key(id_of_currentmatch,category_of_currentmatch,possible_raw_datas[0]));
		assert(itr!=database_info.map_stem->end());
		if (multi_p.abstract_category)
			solution->abstract_categories=itr.value().first;
		else
			solution->abstract_categories=INVALID_BITSET;
		if (multi_p.description)
			solution->description_id=itr.value().second;
		else
			solution->description_id=-1;
		if (multi_p.POS)
			solution->POS=itr.value().third;
		else
			solution->POS="";
		first->indexes.append( AffixPosition(0,itr));
	}
	else
	{
		solution->abstract_categories=INVALID_BITSET;
		solution->description_id=-1;
		solution->POS="";
		first->indexes.append(AffixPosition(0,database_info.map_stem->end()));
	}
	first->store_solution(*solution);
	return first;
}

bool StemSearch::computeNextSolution(solution_position * current)//compute next posibility
{
	if (multi_p.NONE())
		return false;
	SolutionsCompare comp(multi_p);
	if (!multi_p.raw_dataONLY())
	{
		ItemCatRaw2PosDescAbsMapItr & itr=current->indexes[0].second;
		itr++;
		QString raw_data=possible_raw_datas[current->indexes[0].first];
		Map_key key=itr.key();
		if (itr == database_info.map_stem->end() || key != Map_key(id_of_currentmatch,category_of_currentmatch,raw_data) )
		{
			if (current->indexes[0].first<possible_raw_datas.count()-1)//check for next time
			{
				current->indexes[0].first++;
				solution->type=STEM;
				if (multi_p.raw_data)
					solution->raw_data=possible_raw_datas[current->indexes[0].first];
				else
					solution->raw_data="";
				solution->category_id=category_of_currentmatch;
				itr = database_info.map_stem->find(Map_key(id_of_currentmatch,category_of_currentmatch,possible_raw_datas[current->indexes[0].first]));
			}
			else
			{
				//current->clear_stored_solutions();
				return false;
			}
		}
		if (multi_p.abstract_category)
			solution->abstract_categories=itr.value().first;
		else
			solution->abstract_categories=INVALID_BITSET;
		if (multi_p.description)
			solution->description_id=itr.value().second;
		else
			solution->description_id=-1;
		if (multi_p.POS)
			solution->POS=itr.value().third;
		else
			solution->POS="";
		//compare to previous solutions
		if (comp.found(current,*solution))
			return computeNextSolution(current);
		current->store_solution(*solution);
	}
	else
	{
		if (current->indexes[0].first<possible_raw_datas.count()-1)
		{
			current->indexes[0].first++;
			solution->type=STEM;
			solution->raw_data=possible_raw_datas[current->indexes[0].first];
			solution->category_id=category_of_currentmatch;
			solution->abstract_categories=INVALID_BITSET;
			solution->description_id=-1;
			solution->POS="";
		}
		else
		{
			//current->clear_stored_solutions();
			return false;
		}
		//compare to previous solutions
		if (comp.found(current,*solution))
			return computeNextSolution(current);
		current->store_solution(*solution);
	}
	return true;
}
