#include <ostream>
#include <vector>
#include <iterator>
#include <functional>



template < typename T>
using print_function = std::function <void ( std::ostream &, T ) >;

template < typename T>
void ptr_print_function ( std::ostream & o, T const * ptr )
{
	o << *ptr;
}

template < typename InputIt >
std::ostream & to_csv ( std::ostream &o,
		InputIt first,
		InputIt last,
		print_function <typename InputIt::value_type> print,
		const std::string & delim)
{
	auto d = std::distance ( first, last );
	if ( d <= 0 )
		return o;

	auto stop = first;
	std::advance ( stop, d - 1 );

	for ( auto it = first; it != stop; it++ )
	{
		print ( o, *it );
		o << delim;
	}

	print ( o, *stop );

	return o;
}

template < typename InputIt,
	std::ostream & Print ( std::ostream &, typename InputIt::value_type ) >
std::ostream & to_csv ( std::ostream &o, InputIt first, InputIt last )
{
	return to_csv ( o, first, last, Print, ", " );
}
