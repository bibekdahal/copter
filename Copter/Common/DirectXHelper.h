#pragma once

#include <ppltasks.h>	// For create_task

namespace DX
{

	Platform::Array<byte>^ ReadData(Platform::String^ filename);

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> std::vector<byte> 
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
			);

		return SUCCEEDED(hr);
	}
#endif
}

using namespace Windows::ApplicationModel;
ref class RandomAccessReader
{
internal:
    RandomAccessReader(_In_ Platform::String^ fileName)
    {
        Platform::String^ fullPath = Package::Current->InstalledLocation->Path + "\\" + fileName;
        HANDLE fileHandle = ::CreateFile2(fullPath->Data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE)
            throw ref new Platform::FailureException();
        m_file.Attach(fileHandle);
    }

    Platform::Array<byte>^ Read(_In_ size_t bytesToRead)
    {
        Platform::Array<byte>^ fileData = ref new Platform::Array<byte>(static_cast<UINT>(bytesToRead));
        if (!ReadFile(m_file.Get(), fileData->Data, fileData->Length, nullptr, nullptr))
            throw ref new Platform::FailureException();
        return fileData;
    }

    void SeekRelative(_In_ int64 offset)
    {
        LARGE_INTEGER position;
        position.QuadPart = offset;
        BOOL result = ::SetFilePointerEx(m_file.Get(), position, nullptr, FILE_CURRENT);
        if (result == 0)
            throw ref new Platform::FailureException();
    }
    void SeekAbsolute(_In_ int64 position)
    {
        if (position < 0)
            throw ref new Platform::FailureException();
        LARGE_INTEGER pos;
        pos.QuadPart = position;
        BOOL result = ::SetFilePointerEx(m_file.Get(), pos, nullptr, FILE_BEGIN);
        if (result == 0)
            throw ref new Platform::FailureException();
    }
    void SeekToStart()
    {
        LARGE_INTEGER zero = { 0 };
        BOOL result = ::SetFilePointerEx(m_file.Get(), zero, nullptr, FILE_BEGIN);
        if (result == 0)
            throw ref new Platform::FailureException();
    }
    void SeekToEnd()
    {
        LARGE_INTEGER zero = { 0 };
        BOOL result = ::SetFilePointerEx(m_file.Get(), zero, nullptr, FILE_END);
        if (result == 0)
            throw ref new Platform::FailureException();
    }

    uint64 GetFileSize()
    {
        FILE_STANDARD_INFO fileStandardInfo = { 0 };
        BOOL result = ::GetFileInformationByHandleEx(m_file.Get(), FileStandardInfo, &fileStandardInfo, sizeof(fileStandardInfo));
        if ((result == 0) || (fileStandardInfo.AllocationSize.QuadPart < 0))
            throw ref new Platform::FailureException();
        return fileStandardInfo.AllocationSize.QuadPart;
    }
    uint64 GetPosition()
    {
        LARGE_INTEGER position = { 0 }; LARGE_INTEGER zero = { 0 };
        BOOL result = ::SetFilePointerEx(m_file.Get(), zero, &position, FILE_CURRENT);
        if ((result == 0) || (position.QuadPart < 0))
            throw ref new Platform::FailureException();
        return position.QuadPart;
    }

private:
    Microsoft::WRL::Wrappers::FileHandle    m_file;
};

