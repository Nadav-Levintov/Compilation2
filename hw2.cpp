#include "hw2.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <assert.h>
using namespace std;

#define ERROR_CODE -1

vector<bool> compute_nullable_aux()
{
	vector<bool> nullables_bool = vector<bool>(NONTERMINAL_ENUM_SIZE, false);
	set<int> nullables = set<int>();
	set<int> nullables_temp = set<int>();

	for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
	{
		if ((*it).rhs.empty())
		{
			nullables.insert((*it).lhs);
		}
	}

	while (nullables != nullables_temp)
	{
		nullables_temp = nullables;
		for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
		{
			if (!(*it).rhs.empty())
			{
				bool is_nullable = true;
				for (vector<int>::iterator rhs_it = (*it).rhs.begin(); rhs_it != (*it).rhs.end(); rhs_it++)
				{
					/* RHS has a terminal so it is not nullable*/
					if ((*rhs_it) > NONTERMINAL_ENUM_SIZE)
					{
						is_nullable = false;
						break;
					}
					/* RHS has a nonterminal that is not in the set so it is not nullable in this iteration*/
					if (nullables_temp.find(*rhs_it) == nullables_temp.end())
					{
						is_nullable = false;
						break;
					}
				}

				if (is_nullable)
				{
					nullables.insert((*it).lhs);
				}
			}
		}

	}

	for (int i = 0; i < NONTERMINAL_ENUM_SIZE; i++)
	{
		if (nullables.find(i) != nullables.end())
		{
			nullables_bool[i] = true;
		}
	}

	return nullables_bool;
}

void compute_nullable()
{
	vector<bool> nullables_bool = compute_nullable_aux();

	print_nullable(nullables_bool);

}

bool is_token_nullable(int t, vector<bool>& nullables_bool)
{
	if (t > NONTERMINAL_ENUM_SIZE)
		return false;

	return nullables_bool[t];
}

vector< std::set<tokens> > compute_first_aux() {
	vector<bool> nullables_bool = compute_nullable_aux();
	vector< std::set<tokens> > first_all = std::vector< std::set<tokens> >();
	vector< std::set<tokens> > first_all_temp = std::vector< std::set<tokens> >();
	int i = 0;

	/* initialization*/
	for (i = 0; i <= NONTERMINAL_ENUM_SIZE; i++)
	{
		first_all.push_back(std::set<tokens>());
	}
	for (; i <= EF; i++)
	{
		set<tokens> temp = set<tokens>();
		temp.insert(static_cast<tokens>(i));
		first_all.push_back(temp);
	}

	/* step */

	while (first_all != first_all_temp)
	{
		first_all_temp = first_all;
		for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
		{
			vector<int>::iterator rhs_it = (*it).rhs.begin();
			if (rhs_it != (*it).rhs.end())
			{
				first_all[(*it).lhs].insert(first_all_temp[*rhs_it].begin(), first_all_temp[*rhs_it].end());

				while (is_token_nullable(*rhs_it, nullables_bool))
				{
					rhs_it++;
					first_all[(*it).lhs].insert(first_all_temp[*rhs_it].begin(), first_all_temp[*rhs_it].end());
				}
			}
		}

	}


	return first_all;
}

void compute_first() {
	vector< std::set<tokens> > first_non_terminals = std::vector< std::set<tokens> >();
	vector< std::set<tokens> > first_all = compute_first_aux();
	int i = 0;

	vector< std::set<tokens> >::iterator vec_it = first_all.begin();
	for (i = 0; i < NONTERMINAL_ENUM_SIZE; i++)
	{
		first_non_terminals.push_back(*vec_it);
		vec_it++;
	}
	print_first(first_non_terminals);
}

std::vector< std::set<tokens> > compute_follow_aux() {
	vector<bool> nullables_bool = compute_nullable_aux();
	vector< std::set<tokens> > first = compute_first_aux();
	std::vector< std::set<tokens> > follow = std::vector< std::set<tokens> >();
	std::vector< std::set<tokens> > follow_temp = std::vector< std::set<tokens> >();
	int i = 0;

	/* initialization*/
	for (i = 0; i < NONTERMINAL_ENUM_SIZE; i++)
	{
		follow.push_back(std::set<tokens>());
	}
	follow[S].insert(EF);

	/* step */

	while (follow != follow_temp)
	{
		follow_temp = follow;

		for (i = 0; i < NONTERMINAL_ENUM_SIZE; i++)
		{
			for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
			{
				vector<int>::iterator rhs_it = (*it).rhs.begin();
				vector<int>::iterator rhs_end = (*it).rhs.end();
				if (rhs_it != rhs_end && std::find(rhs_it, rhs_end, i) != rhs_end)
				{
					while (rhs_it != rhs_end)
					{
						if ((*rhs_it) == i) {
							rhs_it++;
							if (rhs_it != rhs_end)
							{
								follow[i].insert(first[*rhs_it].begin(), first[*rhs_it].end());
								if (is_token_nullable(*rhs_it, nullables_bool))
								{
									follow[i].insert(follow_temp[(*it).lhs].begin(), follow_temp[(*it).lhs].end());
								}
							}
							else
							{
								follow[i].insert(follow_temp[(*it).lhs].begin(), follow_temp[(*it).lhs].end());
								break;
							}
						}
						rhs_it++;
					}
				}
			}

		}
	}
	return follow;
}



void compute_follow() {
	print_follow(compute_follow_aux());
}

set<tokens> compute_first_rhs(vector<int> vec, const vector< std::set<tokens> > first, const vector<bool> nullable)
{
	set<tokens> res = set<tokens>();
	for (vector<int> ::iterator it = vec.begin(); it != vec.end(); it++)
	{
		res.insert(first[*it].begin(), first[*it].end());
		if ((*it) > NONTERMINAL_ENUM_SIZE || !nullable[*it])
		{
			break;
		}
	}

	return res;
}

bool is_rhs_nullable(vector<int> vec, const vector<bool> nullable) {
	for (vector<int> ::iterator it = vec.begin(); it != vec.end(); it++)
	{
		if ((*it) > NONTERMINAL_ENUM_SIZE || !nullable[*it])
			return false;
	}
	return true;
}

vector< std::set<tokens> > compute_select_aux() {
	vector<bool> nullable = compute_nullable_aux();
	vector< std::set<tokens> > first = compute_first_aux();
	vector< std::set<tokens> > follow = compute_follow_aux();
	vector< std::set<tokens> > select = vector< std::set<tokens> >();

	int i = 0;
	for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
	{
		set<tokens> temp = compute_first_rhs((*it).rhs, first, nullable);
		select.push_back(temp);
		if (is_rhs_nullable((*it).rhs, nullable))
		{
			set<tokens> temp2 = follow[(*it).lhs];
			select[i].insert(temp2.begin(), temp2.end());
		}
		i++;
	}

	return select;
}

void compute_select() {
	print_select(compute_select_aux());
}

map<nonterminal, map<tokens, int> > create_map()
{
	vector< std::set<tokens> > select = compute_select_aux();
	map<nonterminal, map<tokens, int> > M = map<nonterminal, map<tokens, int> >();


	for (int i = S; i < NONTERMINAL_ENUM_SIZE; i++)
	{
		map<tokens, int> temp_map = map<tokens, int>();
		for (int j = STARTSTRUCT; j <= EF; j++)
		{
			temp_map.insert(pair<tokens, int>(static_cast<tokens>(j), ERROR_CODE));
		}

		M.insert(pair<nonterminal, map<tokens, int> >(static_cast<nonterminal>(i), temp_map));
	}

	int i = 0;
	for (vector<grammar_rule>::iterator it = grammar.begin(); it != grammar.end(); it++)
	{
		set<tokens> terminal_set = select[i];
		map<tokens, int> token_map = M[(*it).lhs];

		for (set<tokens> ::iterator set_it = terminal_set.begin(); set_it != terminal_set.end(); set_it++)
		{
			assert(token_map[(*set_it)] == ERROR_CODE);
			token_map[(*set_it)] = i;
		}

		M[(*it).lhs] = token_map;

		i++;

	}

	return M;
}

bool match(tokens t, vector<int>& Q)
{
	if (Q.back() == t)
	{
		Q.pop_back();
		return true;
	}
	else
	{
		return false;
	}

}

void print_Q(vector<int>& Q)
{
	cout << "Q:" << endl;
	for (vector<int>::iterator it = Q.end(); it != Q.begin();)
	{
		it--;
		cout << "    " << (*it) << endl;
	}
}


bool predict(tokens t, vector<int>& Q, map<nonterminal, map<tokens, int> >& M)
{
	nonterminal x = static_cast<nonterminal>(Q.back());
	int rule_num = M[x][t];
	if (rule_num == ERROR_CODE)
	{
		return false;
	}

	/* Print rule num */
	cout << rule_num + 1 << endl;

	Q.pop_back();

	grammar_rule rule = grammar[rule_num];
	for (vector<int>::iterator it = rule.rhs.end(); it != rule.rhs.begin();)
	{
		it--;
		Q.push_back(*it);
	}

	return true;

}

void parser() {
	vector<int> Q = vector<int>();
	map<nonterminal, map<tokens, int> > M = create_map();
	tokens curr = static_cast<tokens>(yylex());

	Q.push_back(S);

	while (!Q.empty())
	{
		if (Q.back() < NONTERMINAL_ENUM_SIZE)
		{
			if (!predict(curr, Q, M))
			{
				cout << "Syntax error\n";
				return;
			}
		}
		else if (Q.back() <= EF)
		{
			if (!match(curr, Q))
			{
				cout << "Syntax error\n";
				return;
			}

			curr = static_cast<tokens>(yylex());
		}
		else
		{
			break;
		}
	}

	if (EF == curr)
	{
		cout << "Success\n";
	}
	else
	{
		cout << "Syntax error\n";
	}

}
