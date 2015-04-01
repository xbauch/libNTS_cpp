#ifndef NTS_DATA_TYPES_HPP_
#define NTS_DATA_TYPES_HPP_
#pragma once

#include <exception>

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

	};

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
