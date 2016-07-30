//
//  MODULE: Ado2.cpp
//
//	AUTHOR: Carlos Antollini 
//
//  mailto: cantollini@hotmail.com
//
//	Date: 07/02/2003
//
//	Version 2.10
//
// ******************************************************************************
//
// Implementation for this class-library by Jan van den Baard. For the 
// implementation I have made a few changes in respect to the original
// version:
//
//	1) Made the classes UNICODE aware. They should compile into a UNICODE
//	   aware app OK now.
//	2) Moved the ctor and dtor code out of the header (.h) file and into the
//	   code (.cpp) file.
//	3) Added some extra error checking throughout the code.
//	4) All classes are inherited from "ClsADOBase" now. This is a simple
//	   base class which encapsulates some shared functionality.
//	5) Removed ::CoInitialize() and ::CoUninitialize() calls. Client code
//	   should take care of that.
//	6) Code now compiles under Warning level 4 without warnings.
//	7) Fixed some problems from the original classes like memory leaks, no
//	   error handling in the class constructors etc.
//
// 02-Nov-2003 (JBa) - Initial ClassLib version.
// 09-Jul-2004 (JBa) - Added ClsADODatabase::OpenPrompt(). It prompts the user via the Datalinks
//		       selection dialog. Requires ADO at least 2.1.

#define _IMPORT_ADO_
#include "ado2.h"

// Import to enable opening the Datalink selection dialog.
// Requires at least ADO 2.1!
#import "C:\Program Files\Common Files\System\ole db\oledb32.dll" rename_namespace( "dlinks" ) no_auto_exclude
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE 
static char THIS_FILE[] = __FILE__;
#endif  

#define ChunkSize 100

///////////////////////////////////////////////////////
//
// ClsJetEngine Class
//
//BOOL ClsJetEngine::CCompactDatabase( ClsString strDatabaseSource, ClsString strDatabaseDestination )
//{
//	try
//	{
//		IJetEnginePtr jet( __uuidof( JetEngine ));
//		return ( BOOL )( jet->CompactDatabase( _bstr_t( strDatabaseSource ), _bstr_t( strDatabaseDestination )) == S_OK );
//	}
//	catch( _com_error &e ) 
//	{       
//		e;
//		return FALSE;
//	} 
//}

///////////////////////////////////////////////////////
//
// ClsADODatabase Class
//
ClsADODatabase::ClsADODatabase()
{
	m_strClassName		= _T( "ClsADODatabse" );
	m_nRecordsAffected	= 0;
	m_nConnectionTimeout	= 0;
	m_pConnection		= NULL;
	ClsADOBase::CheckHResult( m_pConnection.CreateInstance( __uuidof( Connection )));
}

ClsADODatabase::~ClsADODatabase()
{
	Close();
	if ( m_pConnection ) m_pConnection.Release();
}

DWORD ClsADODatabase::GetRecordCount( _RecordsetPtr m_pRs )
{
	DWORD numRows = ( DWORD )m_pRs->GetRecordCount();
	if( numRows == 0xFFFFFFFF )
	{
		if( m_pRs->EndOfFile != VARIANT_TRUE )
			m_pRs->MoveFirst();

		while( m_pRs->EndOfFile != VARIANT_TRUE )
		{
			numRows++;
			m_pRs->MoveNext();
		}
		if( numRows > 0 )
			m_pRs->MoveFirst();
	}
	return numRows;
}

BOOL ClsADODatabase::OpenPrompt()
{
	_ASSERT_VALID( m_pConnection );
	if( IsOpen())
		Close();

	try 
	{
		// Obtain the IDispatch pointer of the
		// database connection.
		IDispatch *pDispatch = NULL;
		m_pConnection->QueryInterface( IID_IDispatch, ( LPVOID * )&pDispatch );

		// Open the Datalink selection
		// dialog. Preselect the connection
		// that already exists.
		dlinks::IDataSourceLocatorPtr dlPrompt = NULL;
		dlPrompt.CreateInstance( __uuidof( dlinks::DataLinks ));
		if ( dlPrompt->PromptEdit( &pDispatch ))
		{
			// Release the dispatch pointer.
			pDispatch->Release();

			// Setup timeout.
			if( m_nConnectionTimeout != 0 )
				m_pConnection->PutConnectionTimeout( m_nConnectionTimeout );

			// Store the connection string locally.
			m_strConnection = ( LPCTSTR )m_pConnection->ConnectionString;
			return ( BOOL )( m_pConnection->Open( _bstr_t( m_strConnection ), _bstr_t( "" ), _bstr_t( "" ), NULL ) == S_OK );
		}
		// Dialog cancelled!
		pDispatch->Release();
		return FALSE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}	
}

BOOL ClsADODatabase::Open( LPCTSTR lpstrConnection, LPCTSTR lpstrUserID, LPCTSTR lpstrPassword )
{
	_ASSERT_VALID( m_pConnection );
	if( IsOpen())
		Close();

	if( lpstrConnection != NULL && *lpstrConnection != _T( '\0' ))
		m_strConnection = lpstrConnection;

	_ASSERT( ! m_strConnection.IsEmpty());

	try
	{
		if( m_nConnectionTimeout != 0 )
			m_pConnection->PutConnectionTimeout( m_nConnectionTimeout );
		return ( BOOL )( m_pConnection->Open( _bstr_t( m_strConnection ), _bstr_t( lpstrUserID ), _bstr_t( lpstrPassword ), NULL ) == S_OK );
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}	
}


BOOL ClsADODatabase::IsOpen() 
{
	if( m_pConnection )
		return m_pConnection->GetState() != adStateClosed;
	return FALSE;
}

void ClsADODatabase::Close()
{
	if( IsOpen())
		m_pConnection->Close();
}

///////////////////////////////////////////////////////
//
// ClsADORecordset Class
//
ClsADORecordset::ClsADORecordset()
{
	m_strClassName		= _T( "ClsADORecordset" );
	m_pRecordset		= NULL;
	m_pCmd			= NULL;
	m_dwLastError		= 0;
	m_pRecBinding		= NULL;
	m_nEditStatus		= ClsADORecordset::dbEditNone;
	m_nSearchDirection	= ClsADORecordset::searchForward;
	ClsADOBase::CheckHResult( m_pRecordset.CreateInstance( __uuidof( Recordset )));
	ClsADOBase::CheckHResult( m_pCmd.CreateInstance( __uuidof( Command )));
}

ClsADORecordset::ClsADORecordset( ClsADODatabase* pAdoDatabase )
{
	m_strClassName		= _T( "ClsADORecordset" );
	m_pRecordset		= NULL;
	m_pCmd			= NULL;
	m_dwLastError		= 0;
	m_pRecBinding		= NULL;
	m_nEditStatus		= ClsADORecordset::dbEditNone;
	m_nSearchDirection	= ClsADORecordset::searchForward;
	ClsADOBase::CheckHResult( m_pRecordset.CreateInstance( __uuidof( Recordset )));
	ClsADOBase::CheckHResult( m_pCmd.CreateInstance( __uuidof( Command )));
	m_pConnection = pAdoDatabase->GetActiveConnection();
}

ClsADORecordset::~ClsADORecordset()
{
	Close();
	if( m_pRecordset ) m_pRecordset.Release();
	if( m_pCmd       ) m_pCmd.Release();
}

BOOL ClsADORecordset::Open( _ConnectionPtr mpdb, LPCTSTR lpstrExec, int nOption )
{	
	_ASSERT_VALID( m_pRecordset ); 	
	Close();
	
	if ( lpstrExec != NULL && *lpstrExec != _T( '\0' ))
		m_strQuery = lpstrExec;

	_ASSERT( ! m_strQuery.IsEmpty());

	if( m_pConnection == NULL )
		m_pConnection = mpdb;

	//m_strQuery.TrimLeft();
	m_strQuery.Trim();
	BOOL bIsSelect = ( BOOL )( _tcsnicmp( m_strQuery, _T( "Select " ), 7 ) == 0 ); //m_strQuery.Mid( 0, ( int )_tcslen( _T( "Select " ))).CompareNoCase( _T( "Select " )) == 0 && nOption == openUnknown;

	try
	{
		m_pRecordset->CursorType	= adOpenStatic;
		m_pRecordset->CursorLocation	= adUseClient;
		if( bIsSelect || nOption == openQuery || nOption == openUnknown )
			m_pRecordset->Open(_variant_t( m_strQuery ), _variant_t(( IDispatch * )mpdb, TRUE ),
							adOpenStatic, adLockOptimistic, adCmdUnknown );
		else if( nOption == openTable )
			m_pRecordset->Open(_variant_t( m_strQuery ), _variant_t(( IDispatch * )mpdb, TRUE ), 
							adOpenKeyset, adLockOptimistic, adCmdTable );
		else if( nOption == openStoredProc )
		{
			m_pCmd->ActiveConnection	= mpdb;
			m_pCmd->CommandText		= _bstr_t( m_strQuery );
			m_pCmd->CommandType		= adCmdStoredProc;
			m_pConnection->CursorLocation	= adUseClient;
			
			m_pRecordset = m_pCmd->Execute( NULL, NULL, adCmdText );
		}
		else
		{
			TRACE1( _T( "Unknown parameter. %d" ), nOption );
			return FALSE;
		}
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
	return m_pRecordset != NULL;
}

BOOL ClsADORecordset::Open( LPCTSTR lpstrExec, int nOption )
{
	_ASSERT( m_pConnection != NULL );
	_ASSERT( m_pConnection->GetState() != adStateClosed );
	return Open( m_pConnection, lpstrExec, nOption );
}

BOOL ClsADORecordset::OpenSchema( int nSchema, LPCTSTR SchemaID )
{
	try
	{
		_variant_t vtSchemaID = vtMissing;

		if( _tcslen( SchemaID ) != 0 )
			vtSchemaID = SchemaID;
			
		m_pRecordset = m_pConnection->OpenSchema(( enum SchemaEnum )nSchema, vtMissing, vtSchemaID );
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::Requery()
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( IsOpen())
	{
		try
		{
			m_pRecordset->Requery( adExecuteRecord );
		}
		catch( _com_error &e )
		{
			dump_com_error( e );
			return FALSE;
		}
	}
	return TRUE;
}


BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, double& dbValue ) 
{	
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt )
		{
			case VT_R4:
				dbValue = vtFld.fltVal;
				break;

			case VT_R8:
				dbValue = vtFld.dblVal;
				break;

			case VT_DECIMAL:
				// Corrected by Jos� Carlos Mart�nez Gal�n
				dbValue = vtFld.decVal.Lo32;
				dbValue *= ( vtFld.decVal.sign == 128 )? -1 : 1;
				dbValue /= pow(( double )10, ( int )vtFld.decVal.scale ); 
				break;

			case VT_UI1:
				dbValue = vtFld.iVal;
				break;

			case VT_I2:
			case VT_I4:
				dbValue = vtFld.lVal;
				break;

			case VT_INT:
				dbValue = vtFld.intVal;
				break;

			case VT_EMPTY:
			case VT_NULL:
				dbValue = 0.0;
				break;

			default:
				dbValue = vtFld.dblVal;
				break;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, double& dbValue ) 
{	
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		dbValue = 0.0;
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt )
		{
			case VT_R4:
				dbValue = vtFld.fltVal;
				break;

			case VT_R8:
				dbValue = vtFld.dblVal;
				break;

			case VT_DECIMAL:
				//Corrected by Jos� Carlos Mart�nez Gal�n
				dbValue = vtFld.decVal.Lo32;
				dbValue *= ( vtFld.decVal.sign == 128 )? -1 : 1;
				dbValue /= pow(( double )10, ( int )vtFld.decVal.scale ); 
				break;

			case VT_UI1:
				dbValue = vtFld.iVal;
				break;

			case VT_I2:
			case VT_I4:
				dbValue = vtFld.lVal;
				break;

			case VT_INT:
				dbValue = vtFld.intVal;
				break;

			case VT_EMPTY:
			case VT_NULL:
				dbValue = 0.0;
				break;

			default:
				dbValue = vtFld.dblVal;
				break;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, long& lValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		if( vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY )
			lValue = vtFld.lVal;
		else
			lValue = 0;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, long& lValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	

	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		if( vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY )
			lValue = vtFld.lVal;
		else
			lValue = 0;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, unsigned long& ulValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		if( vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY )
			ulValue = vtFld.ulVal;
		else
			ulValue = 0;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, unsigned long& ulValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	

	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		if( vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY )
			ulValue = vtFld.ulVal;
		else
			ulValue = 0;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, int& nValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt )
		{
			case VT_BOOL:
				nValue = vtFld.boolVal;
				break;

			case VT_I2:
			case VT_UI1:
				nValue = vtFld.iVal;
				break;

			case VT_INT:
				nValue = vtFld.intVal;
				break;

			case VT_NULL:
			case VT_EMPTY:
				nValue = 0;
				break;
			default:
				nValue = vtFld.iVal;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, int& nValue ) 
{
	_ASSERT_VALID( m_pRecordset );

	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt )
		{
			case VT_BOOL:
				nValue = vtFld.boolVal;
				break;

			case VT_I2:
			case VT_UI1:
				nValue = vtFld.iVal;
				break;

			case VT_INT:
				nValue = vtFld.intVal;
				break;

			case VT_NULL:
			case VT_EMPTY:
				nValue = 0;
				break;

			default:
				nValue = vtFld.iVal;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, ClsString& strValue, ClsString strDateFormat ) 
{
	_ASSERT_VALID( m_pRecordset ); 	

	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt ) 
		{
			case VT_R4:
				strValue = DblToStr( vtFld.fltVal );
				break;

			case VT_R8:
				strValue = DblToStr( vtFld.dblVal );
				break;

			case VT_BSTR:
				strValue = vtFld.bstrVal;
				break;

			case VT_I2:
			case VT_UI1:
				strValue = IntToStr( vtFld.iVal );
				break;

			case VT_INT:
				strValue = IntToStr( vtFld.intVal );
				break;

			case VT_I4:
				strValue = LongToStr( vtFld.lVal );
				break;

			case VT_UI4:
				strValue = ULongToStr( vtFld.ulVal );
				break;

			case VT_DECIMAL:
			{
				//Corrected by Jos� Carlos Mart�nez Gal�n
				double val = vtFld.decVal.Lo32;
				val *= ( vtFld.decVal.sign == 128 ) ? -1 : 1;
				val /= pow(( double )10, ( int )vtFld.decVal.scale ); 
				strValue = DblToStr( val );
				break;
			}

			case VT_DATE:
			{
				ClsDateTime dt( vtFld );
				if( strDateFormat.IsEmpty())
					strDateFormat = _T( "%Y-%m-%d %H:%M:%S" );
				strValue = dt.Format( strDateFormat );
				break;
			}

			case VT_CY:
			{
				ClsCurrency cy( vtFld );
				strValue = cy.Format();
				break;
			}

			case VT_BOOL:
				strValue = vtFld.boolVal == VARIANT_TRUE ? _T( "True" ) : _T( "False" );
				break;

			default:
				strValue.Empty();
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, ClsString& strValue, ClsString strDateFormat ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt )
		{
			case VT_R4:
				strValue = DblToStr( vtFld.fltVal );
				break;

			case VT_R8:
				strValue = DblToStr( vtFld.dblVal );
				break;

			case VT_BSTR:
				strValue = vtFld.bstrVal;
				break;

			case VT_I2:
			case VT_UI1:
				strValue = IntToStr( vtFld.iVal );
				break;

			case VT_INT:
				strValue = IntToStr( vtFld.intVal );
				break;

			case VT_I4:
				strValue = LongToStr( vtFld.lVal );
				break;

			case VT_UI4:
				strValue = ULongToStr( vtFld.ulVal );
				break;

			case VT_DECIMAL:
			{
				//Corrected by Jos� Carlos Mart�nez Gal�n
				double val = vtFld.decVal.Lo32;
				val *= ( vtFld.decVal.sign == 128 ) ? -1 : 1;
				val /= pow(( double )10, ( int )vtFld.decVal.scale ); 
				strValue = DblToStr( val );
				break;
			}

			case VT_DATE:
			{
				ClsDateTime dt( vtFld );
				if( strDateFormat.IsEmpty())
					strDateFormat = _T( "%Y-%m-%d %H:%M:%S" );
				strValue = dt.Format( strDateFormat );
				break;
			}

			case VT_CY:
			{
				ClsCurrency cy( vtFld );
				strValue = cy.Format();
				break;
			}

			case VT_BOOL:
				strValue = vtFld.boolVal == VARIANT_TRUE ? _T( "True" ) : _T( "False" );
				break;

			default:
				strValue.Empty();
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, ClsDateTime& time ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt ) 
		{
			case VT_DATE:
			{
				time = vtFld;
				break;
			}

			case VT_EMPTY:
			case VT_NULL:
				time.SetStatus( ClsDateTime::null );
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, ClsDateTime& time ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt ) 
		{
			case VT_DATE:
			{
				time = vtFld;
				break;
			}

			case VT_EMPTY:
			case VT_NULL:
				time.SetStatus( ClsDateTime::null );
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, bool& bValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt ) 
		{
			case VT_BOOL:
				bValue = vtFld.boolVal == VARIANT_TRUE ? true : false;
				break;

			case VT_EMPTY:
			case VT_NULL:
				bValue = false;
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, bool& bValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt ) 
		{
			case VT_BOOL:
				bValue = vtFld.boolVal == VARIANT_TRUE ? true : false;
				break;

			case VT_EMPTY:
			case VT_NULL:
				bValue = false;
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, ClsCurrency& cyValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		switch( vtFld.vt ) 
		{
			case VT_CY:
				cyValue = ( CURRENCY )vtFld.cyVal;
				break;

			case VT_EMPTY:
			case VT_NULL:
			{
				cyValue.SetStatus( ClsCurrency::null );
				break;
			}

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, ClsCurrency& cyValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		switch( vtFld.vt ) 
		{
			case VT_CY:
				cyValue = ( CURRENCY )vtFld.cyVal;
				break;

			case VT_EMPTY:
			case VT_NULL:
			{
				cyValue.SetStatus( ClsCurrency::null );
				break;
			}

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( LPCTSTR lpFieldName, _variant_t& vtValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	try
	{
		vtValue = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::GetFieldValue( int nIndex, _variant_t& vtValue ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		vtValue = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::IsFieldNull( LPCTSTR lpFieldName ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		return vtFld.vt == VT_NULL;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::IsFieldNull( int nIndex ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		return vtFld.vt == VT_NULL;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::IsFieldEmpty( LPCTSTR lpFieldName ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( lpFieldName )->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::IsFieldEmpty( int nIndex ) 
{
	_ASSERT_VALID( m_pRecordset ); 	
	
	try
	{
		_variant_t vtFld = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ))->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::SetFieldEmpty( LPCTSTR lpFieldName )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;
	
	return PutFieldValue( lpFieldName, vtFld );
}

BOOL ClsADORecordset::SetFieldEmpty( int nIndex )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;

	return PutFieldValue( _variant_t(( SHORT )nIndex ), vtFld );
}

DWORD ClsADORecordset::GetRecordCount()
{
	_ASSERT_VALID( m_pRecordset ); 	
	DWORD nRows = ( DWORD )m_pRecordset->GetRecordCount();
	if( nRows == 0xFFFFFFFF )
	{
		nRows = 0;
		if( m_pRecordset->EndOfFile != VARIANT_TRUE )
			m_pRecordset->MoveFirst();
		
		while( m_pRecordset->EndOfFile != VARIANT_TRUE )
		{
			nRows++;
			m_pRecordset->MoveNext();
		}
		if( nRows > 0 )
			m_pRecordset->MoveFirst();
	}
	return nRows;
}

BOOL ClsADORecordset::IsOpen()
{
	if( m_pRecordset != NULL && IsConnectionOpen())
		return m_pRecordset->GetState() != adStateClosed;
	return FALSE;
}

void ClsADORecordset::Close()
{
	if( IsOpen())
	{
		if ( m_nEditStatus != dbEditNone )
		      CancelUpdate();

		m_pRecordset->PutSort( _T("" ));
		m_pRecordset->Close();	
	}
}

BOOL ClsADODatabase::Execute( LPCTSTR lpstrExec )
{
	_ASSERT( m_pConnection != NULL );
	_ASSERT( lpstrExec != NULL && *lpstrExec != _T( '\0' ));

	_variant_t vRecords;
	m_nRecordsAffected = 0;

	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pConnection->Execute( _bstr_t( lpstrExec ), &vRecords, adExecuteNoRecords );
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;	
	}
}

BOOL ClsADORecordset::RecordBinding( CADORecordBinding &pAdoRecordBinding )
{
	_ASSERT_VALID( m_pRecordset ); 	
	m_pRecBinding = NULL;

	// Open the binding interface.
	ClsADOBase::CheckHResult( m_pRecordset->QueryInterface( __uuidof( IADORecordBinding ), ( LPVOID* )&m_pRecBinding ));
	
	// Bind the recordset to class
	ClsADOBase::CheckHResult( m_pRecBinding->BindToRecordset( &pAdoRecordBinding ));
	return TRUE;
}

BOOL ClsADORecordset::GetFieldInfo( LPCTSTR lpFieldName, StrADOFieldInfo* fldInfo )
{
	_ASSERT_VALID( m_pRecordset ); 	
	FieldPtr pField = m_pRecordset->Fields->GetItem( lpFieldName );
	return GetFieldInfo( pField, fldInfo );
}

BOOL ClsADORecordset::GetFieldInfo( int nIndex, StrADOFieldInfo* fldInfo )
{
	_ASSERT_VALID( m_pRecordset ); 	

	FieldPtr pField = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ));
	return GetFieldInfo( pField, fldInfo );
}

BOOL ClsADORecordset::GetFieldInfo( FieldPtr pField, StrADOFieldInfo* fldInfo )
{
	memset( fldInfo, 0, sizeof( StrADOFieldInfo ));
	_tcscpy( fldInfo->m_strName, ( LPCTSTR )pField->GetName());
	fldInfo->m_lDefinedSize = ( long )pField->GetDefinedSize();
	fldInfo->m_nType	= ( SHORT )pField->GetType();
	fldInfo->m_lAttributes	= pField->GetAttributes();
	if( ! IsEof())
		fldInfo->m_lSize = ( long )pField->GetActualSize();
	return TRUE;
}

BOOL ClsADORecordset::GetChunk( LPCTSTR lpFieldName, ClsString& strValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	FieldPtr pField = m_pRecordset->Fields->GetItem( lpFieldName );
	return GetChunk( pField, strValue );
}

BOOL ClsADORecordset::GetChunk( int nIndex, ClsString& strValue )
{
	_ASSERT_VALID( m_pRecordset ); 	

	FieldPtr pField = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ));
	return GetChunk( pField, strValue );
}

BOOL ClsADORecordset::GetChunk( FieldPtr pField, ClsString& strValue )
{
	ClsString str;
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;

	lngSize = ( long )pField->ActualSize;
	
	str.Empty();
	while( lngOffSet < lngSize )
	{ 
		try
		{
			varChunk = pField->GetChunk( ChunkSize );
			
			str       += varChunk.bstrVal;
			lngOffSet += ChunkSize;
		}
		catch( _com_error &e )
		{
			dump_com_error( e );
			return FALSE;
		}
	}

	lngOffSet = 0;
	strValue  = str;
	return TRUE;
}

BOOL ClsADORecordset::GetChunk( LPCTSTR lpFieldName, LPVOID lpData )
{
	_ASSERT_VALID( m_pRecordset ); 	
	FieldPtr pField = m_pRecordset->Fields->GetItem( lpFieldName );
	return GetChunk( pField, lpData );
}

BOOL ClsADORecordset::GetChunk( int nIndex, LPVOID lpData )
{
	_ASSERT_VALID( m_pRecordset ); 	

	FieldPtr pField = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ));
	return GetChunk( pField, lpData );
}

BOOL ClsADORecordset::GetChunk( FieldPtr pField, LPVOID lpData )
{
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;    
	UCHAR chData;
	HRESULT hr;
	long lBytesCopied = 0;

	lngSize = ( long )pField->ActualSize;
	
	while( lngOffSet < lngSize )
	{ 
		try
		{
			varChunk = pField->GetChunk( ChunkSize );

			// Copy the data only upto the Actual Size of Field.  
			for( long lIndex = 0; lIndex <= ( ChunkSize - 1 ); lIndex++ )
			{
				hr = SafeArrayGetElement( varChunk.parray, &lIndex, &chData );
				if( SUCCEEDED( hr ))
				{
					// Take BYTE by BYTE and advance Memory Location
					// hr = SafeArrayPutElement(( SAFEARRAY FAR * )lpData, &lBytesCopied ,&chData ); 
					(( UCHAR * )lpData )[ lBytesCopied ] = chData;
					lBytesCopied++;
				}
				else
					break;
			}
			lngOffSet += ChunkSize;
		}
		catch( _com_error &e )
		{
			dump_com_error( e );
			return FALSE;
		}
	}

	lngOffSet = 0;
	return TRUE;
}

BOOL ClsADORecordset::AppendChunk( LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes )
{
	_ASSERT_VALID( m_pRecordset ); 	
	FieldPtr pField = m_pRecordset->Fields->GetItem( lpFieldName );
	return AppendChunk( pField, lpData, nBytes );
}

BOOL ClsADORecordset::AppendChunk( int nIndex, LPVOID lpData, UINT nBytes )
{
	_ASSERT_VALID( m_pRecordset ); 	
	FieldPtr pField = m_pRecordset->Fields->GetItem( _variant_t(( SHORT )nIndex ));
	return AppendChunk( pField, lpData, nBytes );
}

BOOL ClsADORecordset::AppendChunk( FieldPtr pField, LPVOID lpData, UINT nBytes )
{
	HRESULT        hr;
	_variant_t     varChunk;
	long	       lngOffset = 0;
	UCHAR	       chData;
	SAFEARRAY FAR *psa = NULL;
	SAFEARRAYBOUND rgsabound[ 1 ];

	try
	{
		// Create a safe array to store the array of BYTES 
		rgsabound[ 0 ].lLbound = 0;
		rgsabound[ 0 ].cElements = nBytes;
		psa = SafeArrayCreate( VT_UI1, 1, rgsabound );
		if ( psa )
		{
			while( lngOffset < (long)nBytes )
			{
				chData	= (( UCHAR * )lpData )[ lngOffset ];
				hr = SafeArrayPutElement( psa, &lngOffset, &chData );
				if( FAILED( hr ))
					return FALSE;
				
				lngOffset++;
			}
		}
		else
			return FALSE;
		lngOffset = 0;

		// Assign the Safe array  to a variant. 
		varChunk.vt     = VT_ARRAY | VT_UI1;
		varChunk.parray = psa;
		hr = pField->AppendChunk( varChunk );
		if( SUCCEEDED( hr )) return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
	return FALSE;
}

ClsString ClsADORecordset::GetString( LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_bstr_t varOutput;
	_bstr_t varNull;
	_bstr_t varCols("\t");
	_bstr_t varRows("\r");

	if ( _tcslen( lpCols ) != 0 )
		varCols = _bstr_t( lpCols );

	if ( _tcslen( lpRows ) != 0 )
		varRows = _bstr_t( lpRows );
	
	if ( numRows == 0 )
		numRows = ( long )GetRecordCount();			
			
	varOutput = m_pRecordset->GetString( adClipString, numRows, varCols, varRows, varNull );
	//return ( LPCTSTR )varOutput;
	return ClsString(( LPCTSTR )varOutput );
}

ClsString IntToStr( int nVal )
{
	TCHAR buff[ 10 ];
	
	_itot( nVal, buff, 10 );
	return ClsString( buff );
}

ClsString LongToStr( long lVal )
{
	TCHAR buff[ 20 ];
	
	_ltot( lVal, buff, 10 );
	return ClsString( buff );
}

ClsString ULongToStr( unsigned long ulVal )
{
	TCHAR buff[ 20 ];
	
	_ultot( ulVal, buff, 10 );
	return ClsString( buff );
}

ClsString DblToStr( double dblVal, int ndigits )
{
	char buff[ _CVTBUFSIZE ];

	// Unicode conversion done by ClsString class.
	_gcvt( dblVal, ndigits, buff );
	return ClsString( buff );
}

ClsString DblToStr( float fltVal )
{
	char buff[ _CVTBUFSIZE ];
	
	// Unicode conversion done by ClsString class.
	_gcvt( fltVal, 10, buff );
	return ClsString( buff );
}

void ClsADORecordset::Edit()
{
	m_nEditStatus = dbEdit;
}

BOOL ClsADORecordset::AddNew()
{
	_ASSERT_VALID( m_pRecordset ); 	
	m_nEditStatus = dbEditNone;
	try
	{
		m_pRecordset->AddNew();
		m_nEditStatus = dbEditNew;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::AddNew( CADORecordBinding &pAdoRecordBinding )
{
	_ASSERT_VALID( m_pRecBinding ); 	
	try
	{
		m_pRecBinding->AddNew( &pAdoRecordBinding );
		m_pRecBinding->Update( &pAdoRecordBinding );
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}	
}

BOOL ClsADORecordset::Update()
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_nEditStatus != dbEditNone )
	{
		try
		{
			m_pRecordset->Update();
			return TRUE;
		}
		catch( _com_error &e )
		{
			m_pRecordset->CancelUpdate();
			dump_com_error( e );
			return FALSE;
		}
	}
	return TRUE;
}

void ClsADORecordset::CancelUpdate()
{
	_ASSERT_VALID( m_pRecordset ); 	
	m_pRecordset->CancelUpdate();
	m_nEditStatus = dbEditNone;
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, const ClsString& strValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t( strValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, const ClsString& strValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t( strValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, int nValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t(( SHORT )nValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, int nValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t(( SHORT )nValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, long lValue )
{
	_ASSERT_VALID( m_pRecordset ); 		
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t( lValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, long lValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t( lValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, unsigned long ulValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t( ulValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, unsigned long ulValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t( ulValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, double dblValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t( dblValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, double dblValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t( dblValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, const ClsDateTime& time )
{
	_ASSERT_VALID( m_pRecordset );
	if ( time.GetStatus() == ClsDateTime::invalid )
		return FALSE;
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t(( DATE )time ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, const ClsDateTime& time )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if ( time.GetStatus() == ClsDateTime::invalid )
		return FALSE;
	return PutFieldValue( lpFieldName, _variant_t(( DATE )time ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, bool bValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t( bValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, bool bValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, _variant_t( bValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, const ClsCurrency& cyValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( cyValue.GetStatus() == ClsCurrency::invalid )
		return FALSE;
	return PutFieldValue( _variant_t(( SHORT )nIndex ), _variant_t(( CURRENCY )cyValue ));
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, const ClsCurrency& cyValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( cyValue.GetStatus() == ClsCurrency::invalid )
		return FALSE;
	return PutFieldValue( lpFieldName, _variant_t(( CURRENCY )cyValue ));
}

BOOL ClsADORecordset::SetFieldValue( int nIndex, const _variant_t& vtValue )
{
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( _variant_t(( SHORT )nIndex ), vtValue );
}

BOOL ClsADORecordset::SetFieldValue( LPCTSTR lpFieldName, const _variant_t& vtValue )
{	
	_ASSERT_VALID( m_pRecordset ); 	
	return PutFieldValue( lpFieldName, vtValue );
}


BOOL ClsADORecordset::SetBookmark()
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_varBookmark.vt != VT_EMPTY )
	{
		m_pRecordset->Bookmark = m_varBookmark;
		return TRUE;
	}
	return FALSE;
}

BOOL ClsADORecordset::Delete()
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_pRecordset->Delete( adAffectCurrent ) != S_OK )
		return FALSE;

	if( m_pRecordset->Update() != S_OK )
		return FALSE;
	
	m_nEditStatus = dbEditNone;
	return TRUE;
}

BOOL ClsADORecordset::Find( LPCTSTR lpFind, int nSearchDirection )
{
	_ASSERT_VALID( m_pRecordset ); 	
	m_strFind = lpFind;
	m_nSearchDirection = nSearchDirection;

	_ASSERT( ! m_strFind.IsEmpty());

	if( m_nSearchDirection == searchForward )
	{
		m_pRecordset->Find( _bstr_t( m_strFind ), 0, adSearchForward, _T( "" ));
		if( ! IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else if( m_nSearchDirection == searchBackward )
	{
		m_pRecordset->Find( _bstr_t( m_strFind ), 0, adSearchBackward, _T( "" ));
		if( ! IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		TRACE1( _T( "Unknown parameter. %d" ), nSearchDirection );
		m_nSearchDirection = searchForward;
	}
	return FALSE;
}

BOOL ClsADORecordset::FindFirst( LPCTSTR lpFind )
{
	_ASSERT_VALID( m_pRecordset ); 	
	m_pRecordset->MoveFirst();
	return Find( lpFind );
}

BOOL ClsADORecordset::FindNext()
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_nSearchDirection == searchForward )
	{
		m_pRecordset->Find( _bstr_t( m_strFind ), 1, adSearchForward, m_varBookFind );
		if( ! IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		m_pRecordset->Find( _bstr_t( m_strFind ), 1, adSearchBackward, m_varBookFind );
		if( ! IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ClsADORecordset::PutFieldValue( LPCTSTR lpFieldName, _variant_t vtFld )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_nEditStatus == dbEditNone )
		return FALSE;
	
	try
	{
		m_pRecordset->Fields->GetItem( lpFieldName )->Value = vtFld; 
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;	
	}
}


BOOL ClsADORecordset::PutFieldValue( _variant_t vtIndex, _variant_t vtFld )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( m_nEditStatus == dbEditNone )
		return FALSE;

	try
	{
		m_pRecordset->Fields->GetItem( vtIndex )->Value = vtFld;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::Clone( ClsADORecordset &pRs )
{
	_ASSERT_VALID( m_pRecordset ); 	
	try
	{
		pRs.m_pRecordset = m_pRecordset->Clone( adLockUnspecified );
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::SetFilter( LPCTSTR strFilter )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_ASSERT( IsOpen());
	
	try
	{
		m_pRecordset->PutFilter( strFilter );
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::SetSort( LPCTSTR strCriteria )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_ASSERT( IsOpen());
	
	try
	{
		m_pRecordset->PutSort( strCriteria );
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::SaveAsXML( LPCTSTR lpstrXMLFile )
{
	_ASSERT_VALID( m_pRecordset ); 	
	_ASSERT( IsOpen());
	
	try
	{
		return ( BOOL )( m_pRecordset->Save( lpstrXMLFile, adPersistXML ) == S_OK );
	}
	catch(_com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
	//return TRUE;
}

BOOL ClsADORecordset::OpenXML( LPCTSTR lpstrXMLFile )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( IsOpen())
		Close();

	try
	{
		return ( BOOL )( m_pRecordset->Open( lpstrXMLFile, _T( "Provider=MSPersist;" ), adOpenForwardOnly, adLockOptimistic, adCmdFile ) == S_OK );
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADORecordset::Execute( ClsADOCommand* pAdoCommand )
{
	_ASSERT_VALID( m_pRecordset ); 	
	if( IsOpen())
		Close();

	_ASSERT( ! pAdoCommand->GetText().IsEmpty());

	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pRecordset = pAdoCommand->GetCommand()->Execute( NULL, NULL, pAdoCommand->GetType());
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

///////////////////////////////////////////////////////
//
// ClsADOCommand Class
//
ClsADOCommand::ClsADOCommand( ClsADODatabase* pAdoDatabase, const ClsString& strCommandText, int nCommandType )
{
	m_strClassName			= _T( "ClsADOCommand" );
	m_pCommand			= NULL;
	ClsADOBase::CheckHResult( m_pCommand.CreateInstance( __uuidof( Command )));
	m_strCommandText		= strCommandText;
	m_pCommand->CommandText		= ( LPCTSTR )m_strCommandText;
	m_nCommandType			= nCommandType;
	m_pCommand->CommandType		= ( CommandTypeEnum )m_nCommandType;
	m_pCommand->ActiveConnection	= pAdoDatabase->GetActiveConnection();	
	m_nRecordsAffected		= 0;
}

ClsADOCommand::~ClsADOCommand()
{
	if ( m_pCommand ) m_pCommand.Release();
}

BOOL ClsADOCommand::AddParameter( ClsADOParameter* pAdoParameter )
{
	_ASSERT_VALID( m_pCommand ); 	
	_ASSERT( pAdoParameter->GetParameter() != NULL );

	try
	{
		m_pCommand->Parameters->Append( pAdoParameter->GetParameter());
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, int nValue )
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vtValue;

	vtValue.vt   = VT_I2;
	vtValue.iVal = ( SHORT )nValue;

	return AddParameter( strName, nType, nDirection, lSize, vtValue );
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, long lValue )
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vtValue;

	vtValue.vt   = VT_I4;
	vtValue.lVal = lValue;

	return AddParameter( strName, nType, nDirection, lSize, vtValue );
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision, int nScale)
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vtValue;

	vtValue.vt     = VT_R8;
	vtValue.dblVal = dblValue;

	return AddParameter( strName, nType, nDirection, lSize, vtValue, nPrecision, nScale );
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, const ClsString& strValue )
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vtValue;

	vtValue = strValue;

	return AddParameter( strName, nType, nDirection, lSize, vtValue );
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, const ClsDateTime& time )
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vtValue;

	vtValue.vt   = VT_DATE;
	vtValue.date = time;

	return AddParameter( strName, nType, nDirection, lSize, vtValue );
}

BOOL ClsADOCommand::AddParameter( const ClsString& strName, int nType, int nDirection, long lSize, const _variant_t& vtValue, int nPrecision, int nScale )
{
	_ASSERT_VALID( m_pCommand ); 	
	try
	{
		_bstr_t name( strName );
		_ParameterPtr pParam = m_pCommand->CreateParameter( name.GetBSTR(), ( ADODB::DataTypeEnum )nType, ( ParameterDirectionEnum )nDirection, lSize, vtValue );
		pParam->PutPrecision(( unsigned char )nPrecision );
		pParam->PutNumericScale(( unsigned char )nScale );
		m_pCommand->Parameters->Append( pParam );
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

void ClsADOCommand::SetText( const ClsString& strCommandText )
{
	_ASSERT_VALID( m_pCommand ); 	
	_ASSERT( ! strCommandText.IsEmpty());

	m_strCommandText = strCommandText;
	m_pCommand->CommandText = ( LPCTSTR )m_strCommandText;
}

void ClsADOCommand::SetType( int nCommandType )
{
	_ASSERT_VALID( m_pCommand ); 	
	m_nCommandType = nCommandType;
	m_pCommand->CommandType = ( CommandTypeEnum )m_nCommandType;
}

BOOL ClsADOCommand::Execute()
{
	_ASSERT_VALID( m_pCommand ); 	
	_variant_t vRecords;
	m_nRecordsAffected = 0;
	try
	{
		m_pCommand->Execute( &vRecords, NULL, adCmdStoredProc );
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

///////////////////////////////////////////////////////
//
// ClsADOParameter Class
//
ClsADOParameter::ClsADOParameter( int nType, long lSize, int nDirection, const ClsString& strName)
{
	m_strClassName		= _T( "ClsADOParameter" );
	m_pParameter		= NULL;
	ClsADOBase::CheckHResult( m_pParameter.CreateInstance( __uuidof( Parameter )));
	m_pParameter->Direction = ( ParameterDirectionEnum )nDirection;
	m_strName		= strName;
	m_pParameter->Name	= ( LPCTSTR )m_strName;
	m_pParameter->Type	= ( ADODB::DataTypeEnum )nType;
	m_pParameter->Size	= lSize;
	m_nType			= nType;
}

ClsADOParameter::~ClsADOParameter()
{
	if ( m_pParameter ) m_pParameter.Release();
}

BOOL ClsADOParameter::SetValue( int nValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	_ASSERT( m_pParameter != NULL );
	
	vtVal.vt   = VT_I2;
	vtVal.iVal = ( SHORT )nValue;

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( int );

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::SetValue( long lValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	_ASSERT( m_pParameter != NULL );
	
	vtVal.vt   = VT_I4;
	vtVal.lVal = lValue;

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( long );

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::SetValue( double dblValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	_ASSERT( m_pParameter != NULL );
	
	vtVal.vt     = VT_R8;
	vtVal.dblVal = dblValue;

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( double );

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::SetValue( const ClsString& strValue)
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	_ASSERT( m_pParameter != NULL );
	
	if( strValue.IsEmpty())
		vtVal.vt = VT_NULL;
	else
		vtVal = strValue;

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( TCHAR  ) * strValue.GetStringLength();

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::SetValue( const ClsDateTime& time )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	_ASSERT( m_pParameter != NULL );
	
	vtVal.vt   = VT_DATE;
	vtVal.date = time;

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( DATE );

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::SetValue( const _variant_t& vtValue )
{
	_ASSERT_VALID( m_pParameter ); 	

	try
	{
		if( m_pParameter->Size == 0 )
			m_pParameter->Size = sizeof( VARIANT );
		
		m_pParameter->Value = vtValue;
		return TRUE;
	}
	catch( _com_error &e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( int& nValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;
	int nVal = 0;

	try
	{
		vtVal = m_pParameter->Value;

		switch( vtVal.vt )
		{
			case VT_BOOL:
				nVal = vtVal.boolVal;
				break;

			case VT_I2:
			case VT_UI1:
				nVal = vtVal.iVal;
				break;

			case VT_INT:
				nVal = vtVal.intVal;
				break;

			case VT_NULL:
			case VT_EMPTY:
				nVal = 0;
				break;

			default:
				nVal = vtVal.iVal;
		}	
		nValue = nVal;
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( long& lValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;
	long lVal = 0;

	try
	{
		vtVal = m_pParameter->Value;
		if( vtVal.vt != VT_NULL && vtVal.vt != VT_EMPTY )
			lVal = vtVal.lVal;
		lValue = lVal;
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( double& dbValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;
	double dblVal;
	try
	{
		vtVal = m_pParameter->Value;
		switch( vtVal.vt )
		{
			case VT_R4:
				dblVal = vtVal.fltVal;
				break;

			case VT_R8:
				dblVal = vtVal.dblVal;
				break;

			case VT_DECIMAL:
				// Corrected by Jos� Carlos Mart�nez Gal�n
				dblVal = vtVal.decVal.Lo32;
				dblVal *= ( vtVal.decVal.sign == 128 )? -1 : 1;
				dblVal /= pow(( double )10, ( int )vtVal.decVal.scale ); 
				break;

			case VT_UI1:
				dblVal = vtVal.iVal;
				break;

			case VT_I2:
			case VT_I4:
				dblVal = vtVal.lVal;
				break;

			case VT_INT:
				dblVal = vtVal.intVal;
				break;

			case VT_NULL:
			case VT_EMPTY:
				dblVal = 0;
				break;

			default:
				dblVal = 0;
		}
		dbValue = dblVal;
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( ClsString& strValue, ClsString strDateFormat )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;
	ClsString strVal;

	try
	{
		vtVal = m_pParameter->Value;
		switch( vtVal.vt ) 
		{
			case VT_R4:
				strVal = DblToStr(vtVal.fltVal);
				break;

			case VT_R8:
				strVal = DblToStr(vtVal.dblVal);
				break;

			case VT_BSTR:
				strVal = vtVal.bstrVal;
				break;

			case VT_I2:
			case VT_UI1:
				strVal = IntToStr(vtVal.iVal);
				break;

			case VT_INT:
				strVal = IntToStr(vtVal.intVal);
				break;

			case VT_I4:
				strVal = LongToStr(vtVal.lVal);
				break;

			case VT_DECIMAL:
			{
				// Corrected by Jos� Carlos Mart�nez Gal�n
				double val = vtVal.decVal.Lo32;
				val *= ( vtVal.decVal.sign == 128 ) ? -1 : 1;
				val /= pow(( double )10, ( int )vtVal.decVal.scale ); 
				strVal = DblToStr( val );
				break;
			}

			case VT_DATE:
			{
				ClsDateTime dt( vtVal );

				if( strDateFormat.IsEmpty())
					strDateFormat = _T( "%Y-%m-%d %H:%M:%S" );
				strVal = dt.Format( strDateFormat );
				break;
			}

			case VT_EMPTY:
			case VT_NULL:
				strVal.Empty();
				break;
			
			default:
				strVal.Empty();
				return FALSE;
		}
		strValue = strVal;
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( ClsDateTime& time )
{
	_ASSERT_VALID( m_pParameter ); 	
	_variant_t vtVal;

	try
	{
		vtVal = m_pParameter->Value;
		switch( vtVal.vt ) 
		{
			case VT_DATE:
			{
				ClsDateTime dt( vtVal );
				time = dt;
				break;
			}

			case VT_EMPTY:
			case VT_NULL:
				time.SetStatus( ClsDateTime::null );
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}

BOOL ClsADOParameter::GetValue( _variant_t& vtValue )
{
	_ASSERT_VALID( m_pParameter ); 	
	try
	{
		vtValue = m_pParameter->Value;
		return TRUE;
	}
	catch( _com_error& e )
	{
		dump_com_error( e );
		return FALSE;
	}
}
///////////////////////////////////////////////////////
//
// ClsADOException Class
//
ClsADOException::ClsADOException( int nCause, const ClsString& strErrorString )
{
	m_nCause = nCause;
	m_sDescription = strErrorString;
}

ClsADOException::~ClsADOException()
{
}

int ClsADOException::GetError( int nADOError )
{
	switch ( nADOError )
	{
		case noError:
			return ClsADOException::noError;
			break;

		default:
			return ClsADOException::Unknown;
	}	
}

void ClsThrowADOException( int nADOError, ClsString strErrorString )
{
	throw new ClsADOException( nADOError, strErrorString );
}
