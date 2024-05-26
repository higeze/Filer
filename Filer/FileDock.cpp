//#include "FileDock.h"
//#include "ToolBar.h"
//#include "FilerGridView.h"
//#include "LauncherGridView.h"
//#include "FavoritesGridView.h"
//#include "FilerTabGridView.h"
//
//void CFileDock::OnCreate(const CreateEvt& e)
//{
//	/*******/
//	/* Top */
//	/*******/
//	auto spTopDock = std::make_shared<CDockPanel>(this);
//	spTopDock->Dock.set(DockEnum::TopFix);
//	//Launcher
//	auto spLauncher = std::make_shared<CLauncherGridView>(spTopDock.get());
//	spLauncher->ItemsSource.set(*Launchers);
//	spLauncher->Dock.set(DockEnum::Fill);
//	//Tool bar
//	auto spToolBar = std::make_shared<CToolBar>(spTopDock.get());
//	spToolBar->Dock.set(DockEnum::Right);
//	auto spBtn = std::make_shared<CButton>(spToolBar.get());
//	spBtn->Content.set(m_isPreview?L"Prv":L"Nrm");
//	spBtn->Command.subscribe([this, spBtn]() {
//		m_isPreview = !m_isPreview;
//		spBtn->Content.set(m_isPreview ? L"Prv" : L"Nrm");
//	}, shared_from_this());
//	spBtn->OnCreate(CreateEvt(GetWndPtr(), spToolBar.get(), CRectF()));
//
//	spTopDock->Add(spToolBar, spLauncher);
//
//	/********/
//	/* Fill */
//	/********/
//	auto spTabLeft = std::make_shared<CFilerTabGridView>(this);
//	spTabLeft->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource.set(*Favorites);
//	//spTabLeft->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
//	//	[this, spBottomStatus](auto notify) {
//	//	spBottomStatus->Text.set(notify.all_items);
//	//	InvalidateRect(NULL, FALSE);
//	//}, shared_from_this());
//	spTabLeft->SetIsTabStop(true);
//	spTabLeft->Dock.set(DockEnum::Fill);
//
//
//	/********/
//	/* Right */
//	/********/
//	auto spTabRight = std::make_shared<CFilerTabGridView>(this);
//	spTabRight->GetFilerViewPtr()->GetFavoriteGridPtr()->ItemsSource.set(*Favorites);
//	//spTabRight->GetFilerViewPtr()->GetFileGridPtr()->StatusLog.subscribe(
//	//	[this, spBottomStatus](auto notify) {
//	//	spBottomStatus->Text.set(notify.all_items);
//	//	InvalidateRect(NULL, FALSE);
//	//}, shared_from_this());
//	spTabRight->SetIsTabStop(true);
//	spTabRight->Dock.set(DockEnum::Right);
//
//	SetUpFileGrid(spTabLeft, spTabRight);
//	SetUpFileGrid(spTabRight, spTabLeft);
//
//	Add(spTopDock, spTabRight, spTabLeft);
//
//}
//
//void CFileDock::SetUpSubjectObserver(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer)
//{
//	static std::vector<std::wstring> imageExts = { L".bmp", L".gif", L".ico", L".jpg", L".jpeg", L".png",L".tiff" };
//	static std::vector<std::wstring> previewExts = {L".docx", L".doc", L".xlsx", L".xls", L".ppt", L".pptx"};
//
//	subject->GetFilerViewPtr()->GetFileGridPtr()->SelectedItem.subscribe([this, wp = std::weak_ptr(observer)](const std::shared_ptr<CShellFile>& spFile) {
//
//		if (auto observer = wp.lock()) {
//			if (!m_isPreview) return;
//
//			std::shared_ptr<TabData> spObsData = observer->ItemsSource.get_unconst()->at(*observer->SelectedIndex);
//
//			//Text
//			if (auto spTxtData = std::dynamic_pointer_cast<TextTabData>(spObsData);
//				spTxtData && boost::iequals(spFile->GetPathExt(), L".txt")) {
//				//TODO
//			//PDF
//			} else if (auto spPdfData = std::dynamic_pointer_cast<PdfTabData>(spObsData);
//				spPdfData && boost::iequals(spFile->GetPathExt(), L".pdf")) {
//				std::shared_ptr<CPDFDoc> newDoc;
//				newDoc->Open(spFile->GetPath());
//				spPdfData->Scale.set(-1);
//				spPdfData->VScroll.set(0.f);
//				spPdfData->HScroll.set(0.f);
//				spPdfData->Doc.set(newDoc);
//				//Image
//			} else if (auto spImgData = std::dynamic_pointer_cast<ImageTabData>(spObsData);
//				spImgData && std::any_of(imageExts.cbegin(), imageExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
//				CD2DImage newDoc(spFile->GetPath());
//				spImgData->Scale.set(-1);
//				spImgData->VScroll.set(0.f);
//				spImgData->HScroll.set(0.f);
//				spImgData->Image.set(newDoc);
//				//Preview
//			} else if (auto spPrvData = std::dynamic_pointer_cast<PreviewTabData>(spObsData);
//				spPrvData && std::any_of(previewExts.cbegin(), previewExts.cend(), [ext = spFile->GetPathExt()](const auto& imageExt)->bool { return boost::iequals(ext, imageExt); })) {
//				CShellFile newDoc(spFile->GetPath());
//				//spPrvData->Scale.set(-1);
//				//spPrvData->VScroll.set(0.f);
//				//spPrvData->HScroll.set(0.f);
//				spPrvData->Doc.set(newDoc);
//			}
//		}
//
//	}, shared_from_this());
//
//	subject->GetFilerViewPtr()->GetFileGridPtr()->GetFolderContextMenu().Add(
//		std::make_unique<CMenuSeparator2>(),
//		std::make_unique<CMenuItem2>(L"Add to Favorite", [this, subject, observer]()->void {
//		    auto files = subject->GetFilerViewPtr()->GetFileGridPtr()->GetSelectedFiles();
//			for (auto& file : files) {
//				Favorites.push_back(std::make_shared<CFavorite>(file->GetPath(), L""));
//			}
//			
//			subject->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();
//			observer->GetFilerViewPtr()->GetFavoriteGridPtr()->SubmitUpdate();}),
//		std::make_unique<CMenuItem2>(L"Add to Launcher", [this, pWnd = GetWndPtr()]()->void {
//		    auto files = subject->GetFilerViewPtr()->GetFileGridPtr()->GetSelectedFiles();
//			for (auto& file : files) {
//				GetLauncherPropPtr()->Launchers.push_back(std::make_shared<CLauncher>(file->GetPath(), L""));
//			}
//			m_spLauncher->SubmitUpdate();
//		})
//	);
//
//
//}
