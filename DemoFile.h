#pragma once

#include <string>
#include <string_view>
#include <filesystem>

class DemoFile
{
public:
	DemoFile( std::filesystem::path filepath );
	~DemoFile();

	bool IsValid( void ) const;
	std::string_view GetFilename( void ) const;
	std::string GetErrorString( void ) const;
	const char *GetFileBuffer( void ) const;
	size_t GetFilesize( void ) const;

private:
	DemoFile( const DemoFile & ) = delete;
	DemoFile &operator=( const DemoFile & ) = delete;

	enum class DemoErrorType;

	DemoErrorType GetHeaderError( void ) const;

	std::string m_filebuffer;
	size_t m_filesize;
	std::string m_filename;
	DemoErrorType m_error;
};