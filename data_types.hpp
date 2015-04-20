#ifndef NTS_DATA_TYPES_HPP_
#define NTS_DATA_TYPES_HPP_
#pragma once

#include <exception>
#include <ostream>
#include <vector>

namespace nts
{
	class ScalarType
	{
		public:
			enum class Type
			{
				None, // For default constructor
				Integer,
				Real,      // Not supported yet
				BitVector, // And bool as bitvector(1)
				// Not really a type, but type class
				Integral
			};

		private:
			Type _type;
			unsigned int _bitwidth; // only for BitVectors
		
			ScalarType ( Type t, unsigned int bw = 0 );

		public:
			ScalarType ();

			static ScalarType Integer();
			static ScalarType Real();
			static ScalarType Integral();
			static ScalarType BitVector ( unsigned int bitwidth );
			static ScalarType Bool();

			bool is_bitvector () const;
			bool is_integral () const;

			bool operator== ( const ScalarType & t ) const;
			bool operator!= ( const ScalarType & rhs ) const;

			// This makes sense only if the type is BitVector
			unsigned int bitwidth() const { return _bitwidth; }


			void print ( std::ostream & o ) const;
	};

	/*
	 * Each type is an n-dimensional array
	 * (of references to k-dimensional array)
	 * of some scalar type t.
	 *
	 * Comparision of array types ( i.e. types with n > 0)
	 * is not supported and always returns false.
	 */
	class Term;
	class DataType
	{
		private:
			ScalarType _type;
			unsigned int _dim_ref;
			std::vector<Term *> _arr_size;
			
		public:
			DataType ();
			DataType ( ScalarType t,
					unsigned int dim_ref = 0,
					std::vector < Term * > && arr_size = {});

			DataType ( const DataType & orig );
			DataType ( DataType && old );

			~DataType ();

			DataType & operator= ( const DataType & orig );
			DataType & operator= ( DataType && old );
			
			bool operator== ( const DataType & t ) const;
			bool operator!= ( const DataType & t ) const;


			bool is_scalar() const noexcept;
			//void print ( std::ostream & o ) const;
			ScalarType & scalar_type() noexcept { return _type; }
			const ScalarType & scalar_type() const noexcept { return _type; }

			// prints array part of type
			// a[5][4][] : int;
			//  ^^^^^^^^
			//     \-this part of declaration
			void print_arr ( std::ostream & o ) const;

			unsigned int arr_dimension() const { return _arr_size.size(); }
			unsigned int ref_dimension() const { return _dim_ref; }

			const std::vector < Term * > & idx_terms() const { return _arr_size; }

	};

	/**
	 * @brief Find some type t such that t1 and t2 can be coerced to t.
	 * @return true if t was found. t is then stored in result.
	 */
	bool coerce ( const ScalarType & t1, const ScalarType &t2, ScalarType & result) noexcept;
	bool coerce ( const DataType & t1, const DataType & t2, DataType & result ) noexcept;

	ScalarType coerce ( const ScalarType & t1, const ScalarType & t2 );
	DataType   coerce ( const DataType   & t1, const DataType   & t2 );

	/**
	 * @brief Is 'from' coercible to 'to' ?
	 */
	bool coercible_ne ( const ScalarType & from, const ScalarType & to ) noexcept;
	void coercible    ( const ScalarType & from, const ScalarType & to );
	bool coercible_ne ( const DataType   & from, const DataType   & to ) noexcept;
	void coercible    ( const DataType   & from, const DataType   & to );


	class TypeError : public std::exception
	{
		public:
			TypeError() {;}
			virtual ~TypeError() {;}
			virtual const char * what() const noexcept override
			{
				return "NTS type error";
			}
	};
};

#endif // NTS_DATA_TYPES_HPP_
