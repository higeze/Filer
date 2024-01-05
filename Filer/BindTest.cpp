#include "FilerBindGridView.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindColumn.h"
#include "ShellFile.h"
#include "RenameInfo.h"

template CBindGridView<std::shared_ptr<CShellFile>>;
template CBindGridView<std::tuple<std::shared_ptr<CShellFile>, RenameInfo>>;
template CFilerBindGridView<std::shared_ptr<CShellFile>>;