//---------------------------------------------------------------------------
// csved_money.cpp
//
// money formatting for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_money.h"
#include "csved_except.h"
#include "csved_ioman.h"
#include "csved_strings.h"
#include "a_collect.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register money  command
//---------------------------------------------------------------------------

static RegisterCommand <MoneyCommand> rc1_(
	CMD_MONEY,
	"format fields as money/currency values"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const MONEY_HELP = {
	"format fields as money/currency values\n"
	"usage: csvfix money [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to apply edits to - default is all fields\n"
	"  -dp chr\tuse character chr as decimal point symbol - default is full-stop\n"
	"  -ts chr\tuse character chr as thousands separator - default is comma\n"
	"  -cs sym\tuse string sym as currency symbol - default is none\n"
	"  -r\t\treplace fields with new format - default is to append fields\n"
	"#ALL"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

MoneyCommand :: MoneyCommand( const string & name,
							const string & desc )
	: Command( name, desc, MONEY_HELP ),
			mDecimalPoint( '.' ), mThouSep( ',' ), mSymbol( "" ) {
	AddFlag( ALib::CommandLineFlag( FLAG_DPOINT, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_KSEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CURSYM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REPLACE, false, 0 ) );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

int MoneyCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		CSVRow out( row );
		for ( unsigned int i = 0; i < row.size(); i++ ) {
			if ( mFields.empty() || ALib::Contains( mFields, i ) ) {
				if ( mReplace ) {
					out[i] = FormatValue( row[i] );
				}
				else {
					out.push_back( FormatValue( row[i] ) );
				}
			}
		}
		io.WriteRow( out );
	}

	return 0;
}

//---------------------------------------------------------------------------

void MoneyCommand :: ProcessFlags( ALib::CommandLine & cmd ) {
	mSymbol = cmd.GetValue( FLAG_CURSYM, "" );
	string fields = cmd.GetValue( FLAG_COLS, "" );
	string dp = cmd.GetValue( FLAG_DPOINT, "." );
	if ( dp.size() != 1 ) {
		CSVTHROW( "Invalid decimal point value" );
	}
	string ts = cmd.GetValue( FLAG_KSEP, "," );
	if ( dp.size() > 1 ) {
		CSVTHROW( "Invalid thousand separator value" );
	}
	mDecimalPoint = dp[0];
	mThouSep = ts == "" ? 0 : ts[0];

	if ( ! fields.empty() ) {
		CommaListToIndex( ALib::CommaList( fields ), mFields );
	}
	mReplace = cmd.HasFlag( FLAG_REPLACE );
}

//----------------------------------------------------------------------------

string MoneyCommand :: FormatValue( const string & v ) const {
	if ( ! ALib::IsNumber( v ) ) {
		return v;
	}
	double m = ALib::ToReal( v );
	std::ostringstream os;
	os << std::fixed << std::setprecision(2) << m;
	return AddSeparators( os.str() );
}


//----------------------------------------------------------------------------

string MoneyCommand :: AddSeparators( const string & v ) const {
	string cents = v.substr( v.size() - 2, 2 );
	string dollars = v.substr( 0, v.size() - 3 );
	string dsep = "";
	int dcount = 0;
	for ( int i = (int) dollars.size() - 1; i >= 0; i-- ) {
		dsep += dollars[i];
		if ( ++dcount == 3 && i != 0 ) {
			if ( mThouSep ) {
				dsep += mThouSep;
			}
			dcount = 0;
		}
	}

	std::reverse( dsep.begin(), dsep.end() );

	std::cout << "[" << dollars << "][" << cents << "][" << dsep << "]\n";
	std::ostringstream os;
	os << mSymbol << dsep << mDecimalPoint << cents;
	return os.str();
}

//----------------------------------------------------------------------------

}	// end namespace

// end
