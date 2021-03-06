/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://gnu.org/licenses>.

*******************************************************************************/

//==============================================================================
// Interfaces
//==============================================================================

#pragma once

//==============================================================================

#include "coreglobal.h"
#include "datastoreinterface.h"
#include "filetypeinterface.h"
#include "solverinterface.h"

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

static const auto InterfacesDataSignature = QStringLiteral("OpenCOR::Core::InterfacesData");

//==============================================================================

class InterfacesData
{
public:
    explicit InterfacesData(const FileTypeInterfaces &pFileTypeInterfaces,
                            const FileTypeInterfaces &pDataStoreFileTypeInterfaces,
                            const SolverInterfaces &pSolverInterfaces,
                            const DataStoreInterfaces &pDataStoreInterfaces);

    FileTypeInterfaces fileTypeInterfaces() const;
    FileTypeInterfaces dataStoreFileTypeInterfaces() const;
    SolverInterfaces solverInterfaces() const;
    DataStoreInterfaces dataStoreInterfaces() const;

private:
    FileTypeInterfaces mFileTypeInterfaces;
    FileTypeInterfaces mDataStoreFileTypeInterfaces;
    SolverInterfaces mSolverInterfaces;
    DataStoreInterfaces mDataStoreInterfaces;
};

//==============================================================================

FileTypeInterfaces CORE_EXPORT fileTypeInterfaces();
FileTypeInterfaces CORE_EXPORT dataStoreFileTypeInterfaces();
SolverInterfaces CORE_EXPORT solverInterfaces();
DataStoreInterfaces CORE_EXPORT dataStoreInterfaces();

//==============================================================================

FileTypeInterface CORE_EXPORT * fileTypeInterface(DataStoreInterface *pDataStoreInterface);
DataStoreInterface CORE_EXPORT * dataStoreInterface(FileTypeInterface *pFileTypeInterface);

//==============================================================================

} // namespace Core
} // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
