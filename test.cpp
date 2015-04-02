#include <vector>
#include "nts.hpp"

using namespace std;
using namespace nts;


struct Example
{
	struct Nts2
	{
		BasicNts basic;
		BitVectorVariable arg_1;
		BitVectorVariable arg_2;
		BitVectorVariable arg_ret;


		Nts2() :
			arg_1   ( "arg_1", 4 ),
			arg_2   ( "arg_2", 8 ),
			arg_ret ( "arg_ret", 16 )
		{
			arg_1.insert_param_in_to ( &basic );
			arg_2.insert_param_in_to ( &basic );
			arg_ret.insert_param_out_to ( &basic );

		}
	};

	// This should call Nts2
	struct Nts1
	{
		BasicNts basic;
		BitVectorVariable var_1;
		BitVectorVariable var_2;
		BitVectorVariable var_3;

		TransitionRule *ctr1;
		Transition *t1;

		Nts1 ( struct Nts2 *n ) :
			var_1   ( "var_1", 4 ),
			var_2   ( "var_2", 8 ),
			var_3   ( "var_3", 16 )
		{
			var_1.insert_to ( &basic );
			var_2.insert_to ( &basic );
			var_3.insert_to ( &basic );
			ctr1 = new CallTransitionRule ( &n->basic, { &var_1, &var_2}, { &var_3 } );
			t1 = new Transition ( *ctr1 );
			t1->insert_to ( & basic );

		}
		~Nts1()
		{
			delete t1;
			// TransitionRule has virtual destructor
			delete ctr1;
		}

	};

	BitVectorVariable bvvar[4];

	BasicNts nb[2];
	Nts toplevel_nts;

	vector < CallTransitionRule *> ctr;
	vector < FormulaTransitionRule *> ftr;
	vector < Transition * > tr;

	Example() :
		bvvar {
			BitVectorVariable ( "var1", 8 ),
			BitVectorVariable ( "var2", 16),
			BitVectorVariable ( "var3", 1 ),
			BitVectorVariable ( "var4", 4 )
		}

	{	
		// tr[0] - call transition
		ctr.push_back ( new CallTransitionRule ( &nb[1], {}, {} ) );
		tr.push_back ( new Transition ( *ctr.back() ) );

		// tr[1] - call transition
		ctr.push_back ( new CallTransitionRule ( &nb[1], {}, {} ) );
		tr.push_back ( new Transition ( *ctr.back() ) );

		// tr[2] - formula transition
		ftr.push_back ( new FormulaTransitionRule ( nullptr ) );
		tr.push_back ( new Transition ( *ftr.back() ) );

		// tr[3] - formula transition
		ftr.push_back ( new FormulaTransitionRule ( nullptr ) );
		tr.push_back ( new Transition ( *ftr.back() ) );


		tr[0]->insert_to ( &nb[0] );
		tr[2]->insert_to ( &nb[0] );
		tr[3]->insert_to ( &nb[0] );
		tr[1]->insert_to ( &nb[0] );


		nb[0].insert_to ( &toplevel_nts );
		nb[1].insert_to ( &toplevel_nts );

		bvvar[0].insert_to ( & toplevel_nts );
		bvvar[1].insert_param_in_to ( & nb[1] );
		bvvar[2].insert_before ( bvvar[1] );

	}

	~Example()
	{
		for ( auto r : ctr )
			delete r;

		for ( auto r : ftr )
			delete r;

		for ( auto t : tr )
			delete t;
	}


	void try_callees()
	{
		auto i = nb[0].callees().begin();

		printf ( "%p == %p\n", tr[0], (*i).transition() );
		i++;
		printf ( "%p == %p\n", tr[1], (*i).transition() );
		i++;
		printf ( "end? %s\n", i == nb[0].callees().end() ? "yes" : "no" );
	}

	void try_callers()
	{
		auto i = nb[1].callers().begin();
		auto e = nb[1].callers().end();
		for (int j = 0; j < 5 && i != e; ++j, ++i )
		{
			printf( "%d: %p\n", j, (*i).transition() );
		}
	}

	void examples()
	{
		Nts2 n2;
		Nts1 n1 ( &n2 );
	}

};

int main ( void )
{
	printf ( "Hello world\n" );

	Example e1;
	e1.try_callees();
	e1.try_callers();
	e1.examples();


	return 0;
}
