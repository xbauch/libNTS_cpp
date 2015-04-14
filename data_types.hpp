#ifndef NTS_DATA_TYPES_HPP_
#define NTS_DATA_TYPES_HPP_
#pragma once

#include <exception>
#include <ostream>

namespace nts
{
	class DataType
	{
		public:
			enum class Type
			{
				Integer,
				Real,      // Not supported yet
				BitVector, // And bool as bitvector(1)
				// Not really a type, but type class
				Integral
			};

		private:
			Type _type;
			unsigned int _bitwidth; // only for BitVectors
			DataType ( Type t, unsigned int bw );

		public:
			static DataType Integer();
			static DataType Real();
			static DataType Integral();
			static DataType BitVector ( unsigned int bitwidth );
			static DataType Bool();

			bool is_bitvector () const;
			bool is_integral () const;

			bool operator== ( const DataType &t ) const;
			bool operator!= ( const DataType & rhs ) const;

			// This makes sense only if the type is BitVector
			unsigned int bitwidth() const { return _bitwidth; }


			void print ( std::ostream & o ) const;

	};

	/**
	 * @brief Find some type t such that t1 and t2 can be coerced to t.
	 * @return true if t was found. t is then stored in result.
	 */
	bool coerce ( const DataType & t1, const DataType & t2, DataType & result ) noexcept;

	DataType coerce ( const DataType & t1, const DataType & t2 );


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
