#include <QtGui>
#include <iostream>

#include "DialogConfigurator.h"
#include "UnigineFileSystem.h"
#include <UnigineEditor.h>
#include <QThreadPool>
#include <QPixmap>

DialogConfigurator::DialogConfigurator(
	QWidget *parent
) : QDialog(parent) {
	// m_nSliderTrianglesValue = 80000;
	// m_nSliderTrianglesValue = 6774;
	m_bWannaUpdate = false;
	m_bInProgress = false;
	m_bRegenerateGeometry = false;
    m_bRegenerateTexture = false;
	m_nSliderTrianglesValue = 500;
	m_nSliderRadius = 2.0f;
	m_nSliderSurfaceDistortion = 0.05f;
	m_nSliderScaleX = 1.0f;
	m_nSliderScaleY = 1.0f;
	m_nSliderScaleZ = 1.0f;
	m_sFilePath1 = m_temporaryDir.filePath("stone.png");
	m_sFilePathHighlighted = m_temporaryDir.filePath("stone_high.png");
	m_nLabelSize = 130;
	m_nLabelValueSize = 50;
	m_nBasicGeometry = 0;
	m_nPointsOfAttraction = 3;
	m_nStrongOfAttraction = 3.14f;
	m_bGenerateMesh = true;
    m_bGenerateMaterial = true;

	// m_nBasicGeometry = 1;
	// m_nPointsOfAttraction = 0;
	// m_nSliderSurfaceDistortion = 0.0f;
	// m_nSliderTrianglesValue = 250;

	m_pAsyncRunGenerator = new AsyncRunGenerator(this);
	m_pAsyncRunGenerator->setAutoDelete(false);
    

	QVBoxLayout *leftLayout = new QVBoxLayout;

	auto *pGeometry = new QLabel("Geometry:");
	leftLayout->addWidget(pGeometry);

	m_pBasicGemometry = new QComboBox();
	m_pBasicGemometry->addItem(tr("Sphere"), 1);
    m_pBasicGemometry->addItem(tr("Cube"), 2);
	connect(m_pBasicGemometry, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxBasicGeometry_Changed(int)));
	leftLayout->addWidget(m_pBasicGemometry);

	leftLayout->addLayout(  createIntSliderParameterUI("Expected triangles: ", &m_nSliderTrianglesValue, 100, 15000));
	leftLayout->addLayout(createFloatSliderParameterUI("Radius: ", &m_nSliderRadius, 0.1, 4.0));
	leftLayout->addLayout(  createIntSliderParameterUI("Attraction Points:", &m_nPointsOfAttraction, 0, 25));
	leftLayout->addLayout(createFloatSliderParameterUI("Attraction Strong: ", &m_nStrongOfAttraction, 1.0, 50.0));
	leftLayout->addLayout(createFloatSliderParameterUI("Surface distortion : ", &m_nSliderSurfaceDistortion, 0.0, 2.0));
	leftLayout->addLayout(createFloatSliderParameterUI("Scale X: ", &m_nSliderScaleX, 0.1, 10.0));
	leftLayout->addLayout(createFloatSliderParameterUI("Scale Y: ", &m_nSliderScaleY, 0.1, 10.0));
	leftLayout->addLayout(createFloatSliderParameterUI("Scale Z: ", &m_nSliderScaleZ, 0.1, 10.0));

	m_pTextureLabel = new QLabel("Texture:");
	leftLayout->addWidget(m_pTextureLabel);

	// texture resolution
	// m_pTextureResolution = new QComboBox();
	// m_pTextureResolution->addItem(tr("128x128"), 128);
    // m_pTextureResolution->addItem(tr("256x256"), 256);
    // m_pTextureResolution->addItem(tr("512x512"), 512);
    // m_pTextureResolution->addItem(tr("1024x1024"), 1024);
    // connect(m_pTextureResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxTextureResolution_Changed(int)));
	// leftLayout->addWidget(m_pTextureResolution);

	m_pTexturesLayout = new QHBoxLayout();

	// m_pPixmapImageOrigin = new QPixmap();
	// m_pPixmapImageHiglighted = new QPixmap();
	m_pImageView = new QLabel();
	m_pImageView->setPixmap(m_pixmapImageHiglighted);
	m_pImageView->setFixedWidth(512);
	m_pImageView->setFixedHeight(512);
	m_pImageView->setScaledContents(true);
	m_pTexturesLayout->addWidget(m_pImageView);
	
	m_pTrianlesList = new QListWidget();
	connect(m_pTrianlesList, SIGNAL(itemSelectionChanged()),this, SLOT(triangles_itemSelectionChanged()));
	m_pTexturesLayout->addWidget(m_pTrianlesList);

	leftLayout->addLayout(m_pTexturesLayout);

	m_pProgress = new QProgressBar(this);
	m_pProgress->setValue(0);
	m_pProgress->setMinimum(0);
	m_pProgress->setMaximum(100);
	leftLayout->addWidget(m_pProgress);

	QHBoxLayout *buttonsLayout = new QHBoxLayout;

	m_pRegenerateButton = new QPushButton(tr("Regenerate"));
	m_pRegenerateButton->setDefault(true);
	connect(m_pRegenerateButton, SIGNAL(clicked()),this, SLOT(regenerateButton_clicked()));
	buttonsLayout->addWidget(m_pRegenerateButton);

	m_pSaveMeshButton = new QPushButton(tr("Save Mesh"));
	connect(m_pSaveMeshButton, SIGNAL(clicked()),this, SLOT(click_saveMesh()));
	buttonsLayout->addWidget(m_pSaveMeshButton);

	buttonsLayout->addWidget(new QWidget);
	buttonsLayout->addStretch();

	m_pCloseButton = new QPushButton(tr("Close"));
	connect(m_pCloseButton, SIGNAL(clicked()),this, SLOT(close()));
	buttonsLayout->addWidget(m_pCloseButton);
	leftLayout->addLayout(buttonsLayout);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(leftLayout);
	// mainLayout->addLayout(rightLayout);
	setLayout(mainLayout);
	// setLayout(leftLayout);
	setWindowTitle(tr("Prototype Stone Generator - Configurator"));
	setFixedWidth(800);
	// setFixedHeight(sizeHint().height());

	connect(this, &DialogConfigurator::signal_generationComplited, this, &DialogConfigurator::slot_generationComplited);
}

void DialogConfigurator::setGenerateMesh(bool bGenerateMesh) {
	m_bGenerateMesh = bGenerateMesh;
}

void DialogConfigurator::setGenerateMaterial(bool bGenerateMaterial) {
	m_bGenerateMaterial = bGenerateMaterial;
	m_pTextureLabel->setVisible(m_bGenerateMaterial);
	// m_pTextureResolution->setVisible(m_bGenerateMaterial);
	m_pImageView->setVisible(m_bGenerateMaterial);
    m_pTrianlesList->setVisible(m_bGenerateMaterial);
}

void DialogConfigurator::sliderInt_valuesChanged(int nNewValue) {
	QObject* obj = sender();
	CustomIntSlider *pSlider = dynamic_cast<CustomIntSlider *>(obj);
	if (pSlider == nullptr) {
	 	std::cerr << "Could not cast" << std::endl;
	 	return;
	}
	pSlider->updateValue(nNewValue);
	m_bRegenerateGeometry = true;
	this->regenerateGeometry();
}

void DialogConfigurator::sliderFloat_valuesChanged(int nNewValue) {
	QObject* obj = sender();
	CustomFloatSlider *pSlider = dynamic_cast<CustomFloatSlider *>(obj);
	if (pSlider == nullptr) {
	 	std::cerr << "Could not cast" << std::endl;
	 	return;
	}
	pSlider->updateValue(float(nNewValue) / 100);
	m_bRegenerateGeometry = true;
	this->regenerateGeometry();
}

void DialogConfigurator::comboboxTextureResolution_Changed(int nNewValue) {
	std::cout << "comboboxTextureResolution_Changed " << nNewValue << std::endl;
}

void DialogConfigurator::comboboxBasicGeometry_Changed(int nNewValue) {
	std::cout << "comboboxBasicGeometry_Changed " << nNewValue << std::endl;
	m_nBasicGeometry = nNewValue;
	m_bRegenerateGeometry = true;
	this->regenerateGeometry();
}

void DialogConfigurator::click_saveMesh() {
	// add to button - save mesh
	m_pDynamicMesh->saveMesh(QString(m_sRandomName + ".mesh").toStdString().c_str());
}

void DialogConfigurator::regenerateButton_clicked() {
	m_bRegenerateGeometry = true;
	this->regenerateGeometry();
}

void DialogConfigurator::createNode() {
	m_sRandomName = "stone_" + QString::number(std::rand() % 10000);

	m_pDynamicMesh = Unigine::ObjectMeshDynamic::create();
	// TODO select position by a camera
	m_pDynamicMesh->setWorldTransform(translate(Unigine::Math::Vec3(0.0f, 0.0f, 2.0f)));
	m_pDynamicMesh->setShowInEditorEnabledRecursive(1);
	m_pDynamicMesh->setSaveToWorldEnabledRecursive(1);
	m_pDynamicMesh->setName(QString(m_sRandomName).toStdString().c_str());
	QVector<Unigine::NodePtr> pNodes;
	pNodes.push_back(m_pDynamicMesh);
	Editor::SelectorNodes *pSelected = Editor::SelectorNodes::createObjectsSelector(pNodes);
	Editor::Selection::setSelector(pSelected);
	
	m_bRegenerateGeometry = true;
	m_bRegenerateTexture = false;

	if (m_bGenerateMaterial) {
		auto mesh_base = Unigine::Materials::findMaterial("mesh_base");
		m_pMaterial = mesh_base->inherit();
		m_pMaterial->setParent(mesh_base);
		std::string sMaterialPath = QString(m_sRandomName + ".mat").toStdString();
		m_pMaterial->setName(QString(m_sRandomName).toStdString().c_str());
		m_pMaterial->setPath(sMaterialPath.c_str());
		m_pMaterial->save();

		m_pDynamicMesh->setMaterial(m_pMaterial, 0);
		
		m_pImage = Unigine::Image::create();
		
		// on first start regenerate all
		m_bRegenerateTexture = true;
	}
	
	this->regenerateGeometry();
}

void DialogConfigurator::regenerateGeometry() {
	if (m_bInProgress) {
		m_bWannaUpdate = true;
		return;
	}
	m_bInProgress = true;
	m_bWannaUpdate = false;
	
	// TODO update process
	m_pProgress->setValue(0);
	m_pProgress->setMinimum(0);
	m_pProgress->setMaximum(m_nSliderTrianglesValue);

	TextureStoneGeneratorConfig texConf;
	texConf.setFilepath(m_sFilePath1);
	m_pAsyncRunGenerator->setTextureStoneGeneratorConfig(texConf);

	StoneGeneratorConfig newConf;
	newConf.setEstimatedExpectedTriangles(m_nSliderTrianglesValue);
	newConf.setPointsOfAttraction(m_nPointsOfAttraction);
	newConf.setStrongOfAttraction(m_nStrongOfAttraction);
	newConf.setRadius(m_nSliderRadius);
	newConf.setSurfaceDistortion(m_nSliderSurfaceDistortion);
	newConf.setScaleX(m_nSliderScaleX);
	newConf.setScaleY(m_nSliderScaleY);
	newConf.setScaleZ(m_nSliderScaleZ);
	newConf.setBasicGeometry(m_nBasicGeometry);

	m_pAsyncRunGenerator->setStoneGeneratorConfig(newConf);
	m_pAsyncRunGenerator->setRegenerateGeometry(m_bRegenerateGeometry);
	m_pAsyncRunGenerator->setRegenerateTexture(m_bRegenerateTexture);
	m_bRegenerateGeometry = false;
    m_bRegenerateTexture = false;

	QThreadPool::globalInstance()->start(m_pAsyncRunGenerator);
}

void DialogConfigurator::generationComplited(QString sDone) {
	emit signal_generationComplited(sDone);
}

void DialogConfigurator::addVertex(int nIndex, StonePoint *p1, StoneTexturePoint &t1, int nSurface) {
	m_pMeshTemp->addVertex(Unigine::Math::vec3(
		p1->x(),
		p1->y(),
		p1->z()
	), nSurface);
	m_pMeshTemp->addTexCoord0(Unigine::Math::vec2(
		t1.x(),
		t1.y()
	), nSurface);
	// m_pMeshTemp->addColor(Unigine::Math::vec4(0,0,0,255), nSurface);
	m_pMeshTemp->addNormal(Unigine::Math::vec3(
	 	p1->getNormalX(), 
	 	p1->getNormalY(),
	 	p1->getNormalZ()
	), nSurface);
	m_pMeshTemp->addIndex(nIndex, nSurface);
}

void DialogConfigurator::showNormal(int nLastIndex, int nIndex, StonePoint *p1, int nSurface) {
	
	std::vector<Unigine::Math::vec3> points;

	points.push_back(Unigine::Math::vec3(
		p1->x(),
		p1->y(),
		p1->z()
	));
	points.push_back(Unigine::Math::vec3(
		p1->x() + p1->getNormalX() + 0.1,
		p1->y() + p1->getNormalY() + 0.1,
		p1->z() + p1->getNormalZ()
	));
	points.push_back(Unigine::Math::vec3(
		p1->x() + p1->getNormalX() - 0.1,
		p1->y() + p1->getNormalY() + 0.1,
		p1->z() + p1->getNormalZ()
	));
	points.push_back(Unigine::Math::vec3(
		p1->x() + p1->getNormalX(),
		p1->y() + p1->getNormalY() - 0.1,
		p1->z() + p1->getNormalZ()
	));

	int nSize = points.size();
	int nKI = nSize*3;
	for (int i = 0; i < nSize; i++) {
		Unigine::Math::vec3 p1 = points[(i+0)%nSize];
		Unigine::Math::vec3 p2 = points[(i+1)%nSize];
		Unigine::Math::vec3 p3 = points[(i+2)%nSize];
		m_pMeshTemp->addVertex(p1, nSurface);
		m_pMeshTemp->addIndex(nLastIndex + nIndex*nKI + i*3 + 0, nSurface);
		m_pMeshTemp->addNormal(Unigine::Math::vec3(0.0, 0.0, 0.0), nSurface);

		m_pMeshTemp->addVertex(p2, nSurface);
		m_pMeshTemp->addIndex(nLastIndex + nIndex*nKI + i*3 + 1, nSurface);
		m_pMeshTemp->addNormal(Unigine::Math::vec3(0.0, 0.0, 0.0), nSurface);

		m_pMeshTemp->addVertex(p3, nSurface);
		m_pMeshTemp->addIndex(nLastIndex + nIndex*nKI + i*3 + 2, nSurface);
		m_pMeshTemp->addNormal(Unigine::Math::vec3(0.0, 0.0, 0.0), nSurface);
	}
}

void DialogConfigurator::slot_generationComplited(QString sDone) {
	Unigine::Log::message("DialogConfigurator::generationComplited start\n");

	if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
		Unigine::Log::error("UnigineEditorPlugin_Python3Scripting::slot_executeRunner Not main thread!!!");
	}
	std::cout << sDone.toStdString() << std::endl;
	
	m_pDynamicMesh->clearVertex();
	m_pDynamicMesh->clearIndices(); // here triangles like some one
	m_pDynamicMesh->flushIndices();
	m_pDynamicMesh->flushVertex();
	auto *pStoneGenerator = m_pAsyncRunGenerator->getStoneGenerator();

	// const std::vector<StonePoint *> &vPoints = pStoneGenerator->points();
	// for (int i = 0; i < vPoints.size(); i++) {
	// 	m_pDynamicMesh->addVertex(Unigine::Math::vec3(
	// 		vPoints[i]->x(),
	// 		vPoints[i]->y(),
	// 		vPoints[i]->z()
	// 	));
	// 	// m_pDynamicMesh->addTexCoord(Unigine::Math::vec4(float(std::rand() % 100) / 100, float(std::rand() % 100) / 100, 0, 0));
	// 	// std::cout << "tx coord uv: " <<
	// 	// 	vPoints[i]->getTextureCoordinateU()
	// 	// 	<< " " << 
	// 	// 	vPoints[i]->getTextureCoordinateV()
	// 	// 	<< std::endl;
	// 	m_pDynamicMesh->addTexCoord(Unigine::Math::vec4(
	// 		vPoints[i]->getTextureCoordinateU(),
	// 		vPoints[i]->getTextureCoordinateV(),
	// 		0,
	// 		0
	// 	));
	// }

	const std::vector<StoneTriangle *> &vTriangles = pStoneGenerator->triangles();
	m_pMeshTemp = Unigine::Mesh::create();
	// int nSurface = m_pMeshTemp->addEmptySurface("0", vTriangles.size()*3, vTriangles.size()*3);
	int nSurface = m_pMeshTemp->addSurface("0");
	// m_pDynamicMesh->addTriangles(vTriangles.size());
	// m_pDynamicMesh->allocateIndices(vTriangles.size()*3);
	std::cout << "Got triangles: " << vTriangles.size() << std::endl;
	int nLastIndex = 0;
	for (int i = 0; i < vTriangles.size(); i++) {
		StoneTriangle *pTriangle = vTriangles[i];
		this->addVertex(i*3 + 0, pTriangle->p1(), pTriangle->t1(), nSurface);
		this->addVertex(i*3 + 1, pTriangle->p2(), pTriangle->t2(), nSurface);
		this->addVertex(i*3 + 2, pTriangle->p3(), pTriangle->t3(), nSurface);
		nLastIndex = i*3 + 2;
	}

	// show normals
	// nLastIndex = nLastIndex + 1;
	// const std::vector<StonePoint *> &vPoints = pStoneGenerator->points();
	// for (int i = 0; i < vPoints.size(); i++) {
	// 	StonePoint *p1 = vPoints[i];
	// 	this->showNormal(nLastIndex, i, vPoints[i], nSurface);
	// }

	m_pMeshTemp->createTangents();
	// // optimize vertex and index buffers, if necessary
	Unigine::MaterialPtr pPrevMeterial = m_pDynamicMesh->getMaterial(0);

	m_pDynamicMesh->setMesh(m_pMeshTemp);
	// return material
	m_pDynamicMesh->setMaterial(pPrevMeterial, 0);

	m_pDynamicMesh->flushIndices();
	m_pDynamicMesh->flushVertex();

	// m_pDynamicMesh->updateIndices();
	// calculate a mesh bounding box for editor I guess
	m_pDynamicMesh->updateBounds();

	// lighting
	// m_pDynamicMesh->updateTangents();


	if (m_pAsyncRunGenerator->getRegenerateTexture()) {
		m_pImage->clear();
		m_pImage->load(m_sFilePath1.toStdString().c_str());
		m_pixmapImageOrigin.load(m_sFilePath1);
		if (m_pImage->isLoaded()) {
			std::cout << "Image loaded" << std::endl;
			// add to button - save texture
			m_pImage->save(QString(m_sRandomName + ".png").toStdString().c_str());
			// m_pImageView->setScaledContents(true);
		}
		// m_pTexture->clear();
		// m_pTexture->setImage(m_pImage);

		int num = m_pMaterial->findTexture("albedo");
		if (num != -1) {
			std::cout << "Set albedo" << std::endl;
			m_pMaterial->setTexturePath(num, QString(m_sRandomName + ".png").toStdString().c_str());

			// m_pMaterial->setTextureImage(num, m_pImage);
		}
		// m_pMaterial->fl
	}

	if (m_bGenerateMaterial) {
		// reload preview texture image
		this->updateTextureImageView("");
	}

	Unigine::Log::message("DialogConfigurator::generationComplited end\n");
	m_bInProgress = false;
	if (m_bWannaUpdate) {
		Unigine::Log::message("regenerate geometry again\n");
		this->regenerateGeometry();
	}
	m_pMeshTemp = nullptr;
}

void DialogConfigurator::triangles_itemSelectionChanged() {
	std::cout << "triangles_itemSelectionChanged" << std::endl;
	QList<QListWidgetItem *> listItems = m_pTrianlesList->selectedItems();

	// auto *pStoneGenerator = m_pAsyncRunGenerator->getStoneGenerator();
	QStringList highllightTriangles;
	for (int i = 0; i < listItems.size(); i++) {
		QListWidgetItem *pItem = listItems.at(i);
		std::cout << pItem->text().toStdString() << std::endl;
		highllightTriangles.append(pItem->text());
		updateTextureImageView(pItem->text());
		return;
	}
}

int DialogConfigurator::normalizeTextureCoordinates(int nWidth, float nVal) {
	int nRet = nWidth * nVal;
	if (nRet > nWidth) {
		nRet = nWidth;
	}
	if (nRet < 0) {
		nRet = 0;
	}
	return nRet;
}

void DialogConfigurator::updateTextureImageView(const QString &sHeighlightTriangle) {
	// reload preview texture image
	m_pixmapImageHiglighted = m_pixmapImageOrigin;

	auto *pStoneGenerator = m_pAsyncRunGenerator->getStoneGenerator();
	const std::vector<StoneTriangle *> &vTriangles = pStoneGenerator->triangles();

	// print uv map
	QPainter painter(&m_pixmapImageHiglighted);
	QBrush brush( 0xffff00 /*Qt::white*/, Qt::SolidPattern);
	QPen pen(0xffff00 /*Qt::white*/);
	painter.setPen(pen);
	painter.setBrush(brush);
	int nImageHeight = m_pixmapImageHiglighted.height();
	int nImageWidth = m_pixmapImageHiglighted.width();
	if (sHeighlightTriangle == "") {
		m_pTrianlesList->clear();
	}
	bool bSaveImage = false;

	for (int i = 0; i < vTriangles.size(); i++) {
		StoneTriangle *pTriangle = vTriangles[i];
		int nX0 = normalizeTextureCoordinates(nImageHeight, pTriangle->t1().x());
		int nY0 = normalizeTextureCoordinates(nImageWidth,  pTriangle->t1().y());
		int nX1 = normalizeTextureCoordinates(nImageHeight, pTriangle->t2().x());
		int nY1 = normalizeTextureCoordinates(nImageWidth,  pTriangle->t2().y());
		int nX2 = normalizeTextureCoordinates(nImageHeight, pTriangle->t3().x());
		int nY2 = normalizeTextureCoordinates(nImageWidth,  pTriangle->t3().y());

		painter.drawLine(nX0, nY0, nX1, nY1);
		painter.drawLine(nX1, nY1, nX2, nY2);
		painter.drawLine(nX2, nY2, nX0, nY0);
		QString sTrianle = 
			"(" + QString::number(nX0) + "," + QString::number(nY0) + ")"
			+ "(" + QString::number(nX1) + "," + QString::number(nY1) + ")"
			+ "(" + QString::number(nX2) + "," + QString::number(nY2) + ")";
		if (sHeighlightTriangle == "") {
			m_pTrianlesList->addItem(sTrianle);
		} else {
			if (sTrianle == sHeighlightTriangle) {
				QPainterPath path;
				// Set pen to this point.
				path.moveTo (nX0, nY0);
				// Draw line from pen point to this point.
				path.lineTo (nX1, nY1);
				// path.moveTo (endPointX1, endPointY1); // <- no need to move
				path.lineTo (nX2, nY2);
				// path.moveTo (endPointX2,   endPointY2); // <- no need to move
				path.lineTo (nX0, nY0);
				// painter.setPen (Qt :: NoPen);
				painter.fillPath (path, QBrush (QColor ("blue")));
				// TODO draw higlighted triangul
				bSaveImage = true;
			}
		}
		
	}
	m_pImageView->setPixmap(m_pixmapImageHiglighted);
	if (bSaveImage) {
		m_pixmapImageHiglighted.save(m_sFilePathHighlighted);
		m_pImage->load(m_sFilePathHighlighted.toStdString().c_str());
		if (m_pImage->isLoaded()) {
			std::cout << "Image loaded" << std::endl;
			// add to button - save texture
			// m_pImage->save(QString(m_sRandomName + ".png").toStdString().c_str());

			int num = m_pMaterial->findTexture("albedo");
			if (num != -1) {
				std::cout << "Set albedo" << std::endl;
				// set runtime
				m_pMaterial->setTextureImage(num, m_pImage);

				// m_pMaterial->setTextureImage(num, m_pImage);
			}
		}
	} else {
		m_pImage->load(m_sFilePath1.toStdString().c_str());
		if (m_pImage->isLoaded()) {
			std::cout << "Image loaded" << std::endl;
			// add to button - save texture
			// m_pImage->save(QString(m_sRandomName + ".png").toStdString().c_str());
			// m_pImageView->setScaledContents(true);
			int num = m_pMaterial->findTexture("albedo");
			if (num != -1) {
				std::cout << "Set albedo (reset picture)" << std::endl;
				// NOT work here
				m_pMaterial->setTexturePath(num, QString(m_sRandomName + ".png").toStdString().c_str());

				// m_pMaterial->setTextureImage(num, m_pImage);
			}
		}
		// m_pTexture->clear();
		// m_pTexture->setImage(m_pImage);
	}

}

QHBoxLayout *DialogConfigurator::createIntSliderParameterUI(QString sLabel, int *nValue, int nMin, int nMax) {
	CustomIntSlider *pSlider = new CustomIntSlider(Qt::Horizontal);

	QHBoxLayout *pLayout = new QHBoxLayout();

	QLabel *pLabel = new QLabel(sLabel);
	pLabel->setFixedWidth(m_nLabelSize);
	pLayout->addWidget(pLabel);

	QLabel *pLabelValue = new QLabel("(" + QString::number(*nValue) + ")");
	pLabelValue->setFixedWidth(m_nLabelValueSize);
	pSlider->setLabelValue(pLabelValue);
	pLayout->addWidget(pLabelValue);
    pSlider->setRange(nMin, nMax);
	pSlider->setValue(*nValue);
    pSlider->setPoiterValue(nValue);
	connect(pSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderInt_valuesChanged(int)));
	pLayout->addWidget(pSlider);
	
	return pLayout;
}

QHBoxLayout *DialogConfigurator::createFloatSliderParameterUI(QString sLabel, float *nValue, float nMin, float nMax) {
	CustomFloatSlider *pSlider = new CustomFloatSlider(Qt::Horizontal);

	QHBoxLayout *pLayout = new QHBoxLayout();

	QLabel *pLabel = new QLabel(sLabel);
	pLabel->setFixedWidth(m_nLabelSize);
	pLayout->addWidget(pLabel);

	QLabel *pLabelValue = new QLabel("(" + QString::number(*nValue) + ")");
	pLabelValue->setFixedWidth(m_nLabelValueSize);
	pSlider->setLabelValue(pLabelValue);
	pLayout->addWidget(pLabelValue);
    pSlider->setRange(nMin*100, nMax*100);
	pSlider->setValue((*nValue)*100);
    pSlider->setPoiterValue(nValue);

	connect(pSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderFloat_valuesChanged(int)));
	pLayout->addWidget(pSlider);
	
	return pLayout;
}