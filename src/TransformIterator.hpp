#ifndef TRANSFORM_ITERATOR_HPP_
#define TRANSFORM_ITERATOR_HPP_
#pragma once

#include <iterator>
#include <functional>


template < typename InputIterator, typename OutputType>
class Mapped
{
	public:
		using Mapper = std::function < OutputType ( typename InputIterator::value_type ) >;

	private:
		InputIterator _begin;
		InputIterator _end;
		const Mapper & _mapper;

	public:
		Mapped ( InputIterator begin, InputIterator end, const Mapper & mapper ) :
			_begin  ( begin ),
			_end    ( end   ),
			_mapper ( mapper )
		{
			;
		}

		class const_iterator
		{
			public:
				using value_type = const OutputType;

			private:
				InputIterator   _it;
				const Mapped  & _mapped;

			public:
				const_iterator ( const InputIterator & it, const Mapped & mapped ) :
					_it     ( it     ),
					_mapped ( mapped )
				{
					;
				}

				value_type operator* () const
				{
					return _mapped._mapper ( *_it );
				}

				bool operator== ( const const_iterator & snd ) const
				{
					return _it == snd._it;
				}

				bool operator!= ( const const_iterator & snd ) const
				{
					return ! operator== ( snd );
				}

				const_iterator & operator++ ()
				{
					_it++;
					return *this;
				}

				const_iterator operator++ ( int )
				{
					const_iterator old = *this;
					++*this;
					return old;
				}
		};

		const_iterator cbegin() const
		{
			return const_iterator ( _begin, *this );
		}

		const_iterator cend() const
		{
			return const_iterator ( _end, *this );
		}
};



#endif // TRANSFORM_ITERATOR_HPP_
