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

	DataType::DataType ( Type t, unsigned int bw) :
		_type ( t ),
		_bitwidth ( bw )
	{
		;
	}

	DataType DataType::Integer()
	{
		return DataType ( Type::Integer, 0 );
	}

	DataType DataType::Real()
	{
		return DataType ( Type::Real, 0 );
	}

	DataType DataType::Integral()
	{
		return DataType ( Type::Integral, 0 );
	}

	DataType DataType::BitVector ( unsigned int bw )
	{
		return DataType ( Type::BitVector, bw );
	}

	DataType DataType::Bool ()
	{
		return BitVector ( 1 );
	}

	bool DataType::is_integral() const
	{
		return _type != Type::Real;
	}

	bool DataType::is_bitvector() const
	{
		return _type == Type::BitVector;
	}

	bool DataType::operator== ( const DataType & rhs ) const
	{
		return _type == rhs._type && _bitwidth == rhs._bitwidth;
	}

	bool DataType::operator!= ( const DataType & rhs ) const
	{
		return ! (*this == rhs);
	}


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
