#ifndef FILTER_ITERATOR_HPP_
#define FILTER_ITERATOR_HPP_

#include <iterator>
#include <functional>

template < typename InputIterator >
class Filtered
{
	private:
		using Predicate = std::function <
			bool ( typename InputIterator::value_type )
			>;

		InputIterator _begin;
		InputIterator _end;
		const Predicate & _p;

	public:

		Filtered ( InputIterator begin, InputIterator end, const Predicate &p ) :
			_begin ( begin ),
			_end   ( end   ),
			_p     ( p     )
		{
			;
		}

		class iterator :
			public std::iterator
			<
				std::input_iterator_tag,
				typename InputIterator::value_type
			>
		{
			private:
				InputIterator     _it;
				InputIterator     _end;
				const Predicate & _p;

				void skip()
				{
					while ( _it != _end && ! _p ( *_it ) )
						++_it;
				}

			public:
				iterator (
						const InputIterator & it,
						const InputIterator & end,
						const Predicate     & p    ) :
					_it  ( it  ),
					_end ( end ),
					_p   ( p   )
				{
					skip();
				}

				typename InputIterator::value_type operator* ()
				{
					return *_it;
				}

				typename InputIterator::value_type operator* () const
				{
					return *_it;
				}

				bool operator== ( const iterator &snd ) const
				{
					return _it == snd._it;
				}

				bool operator!= ( const iterator &snd ) const
				{
					return ! operator== ( snd );
				}

				iterator & operator++ ()
				{
					_it++;
					skip();
					return *this;
				}

				iterator operator++ ( int )
				{
					iterator old = *this;
					++*this;
					return old;
				}

		};


		iterator begin()
		{
			return iterator ( _begin, _end, _p );
		}

		iterator end()
		{
			return iterator ( _end, _end, _p );
		}
};

#endif
