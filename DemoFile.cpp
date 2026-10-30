#include "DemoFile.h"
#include "SharedDefs.h"
#include <fstream>
#include <assert.h>

enum class DemoFile::DemoErrorType
{
	DEMO_NO_ERROR = 0,
	NOT_A_DEMO,
	NOT_A_CSS_V34_DEMO,
	INACCESSIBLE,
	FILE_TOO_SMALL
};

DemoFile::DemoFile( std::filesystem::path filepath )
	: m_error{ DemoErrorType::DEMO_NO_ERROR }, m_filesize{ 0 }
{
	m_filebuffer.clear();
	m_filename = filepath.filename().string();

	if ( !filepath.has_extension() || filepath.extension().string() != ".dem" )
	{
		m_error = DemoErrorType::NOT_A_DEMO;
		return;
	}

	std::ifstream file( filepath, std::ios::binary );

	if ( !file.is_open() )
	{
		m_error = DemoErrorType::INACCESSIBLE;
		return;
	}

	file.seekg( 0, std::ios_base::end );
	m_filesize = (size_t)file.tellg();
	file.seekg( 0, std::ios_base::beg );

	if ( m_filesize > 0 )
	{
		m_filebuffer.resize( m_filesize );
		file.read( &m_filebuffer[ 0 ], m_filesize );
	}
	else
	{
		m_error = DemoErrorType::FILE_TOO_SMALL;
		return;
	}

	m_error = GetHeaderError();
}

DemoFile::~DemoFile()
{

}

bool DemoFile::IsValid( void ) const
{
	return m_error == DemoErrorType::DEMO_NO_ERROR;
}

std::string_view DemoFile::GetFilename( void ) const
{
	return m_filename;
}

std::string DemoFile::GetErrorString( void ) const
{
	switch ( m_error )
	{
		case DemoErrorType::DEMO_NO_ERROR:
			return "fine";
		case DemoErrorType::NOT_A_DEMO:
			return "not a demo file";
		case DemoErrorType::NOT_A_CSS_V34_DEMO:
			return "not a CS:S v34 demo file";
		case DemoErrorType::INACCESSIBLE:
			return "inaccessible";
		case DemoErrorType::FILE_TOO_SMALL:
			return "too small";
		default:
			return "";
	}
}

const char *DemoFile::GetFileBuffer( void ) const
{
	return &m_filebuffer[ 0 ];
}

size_t DemoFile::GetFilesize( void ) const
{
	return m_filesize;
}

DemoFile::DemoErrorType DemoFile::GetHeaderError( void ) const
{
	constexpr auto DEMO_FILESTAMP = "HL2DEMO";
	constexpr auto DEMO_PROTOCOL = 3;
	constexpr auto NETWORK_PROTOCOL = 7;
	constexpr auto CS_GAMEDIR = "cstrike";

	if ( m_filesize < sizeof( demoheader_t ) )
	{
		return DemoErrorType::FILE_TOO_SMALL;
	}

	const demoheader_t *hdr = (demoheader_t *)m_filebuffer.c_str();

	if ( strcmp( hdr->demofilestamp, DEMO_FILESTAMP ) )
	{
		return DemoErrorType::NOT_A_DEMO;
	}

	if ( hdr->demoprotocol != DEMO_PROTOCOL
	|| hdr->networkprotocol != NETWORK_PROTOCOL
	|| strcmp( hdr->gamedirectory, CS_GAMEDIR ) )
	{
		return DemoErrorType::NOT_A_CSS_V34_DEMO;
	}

	return DemoErrorType::DEMO_NO_ERROR;
}