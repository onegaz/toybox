#include <QtWidgets>
#include "imageviewer.hpp"

ImageViewer::ImageViewer(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    m_image = reader.read();
    if (m_image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return;
    }

	m_imageLabel = new QLabel(this);
	m_imageLabel->setPixmap(QPixmap::fromImage(m_image));
	m_imageLabel->setBackgroundRole(QPalette::Base);
	m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_imageLabel->setScaledContents(true);
	m_scrollArea = new QScrollArea{this};
	m_scrollArea->setBackgroundRole(QPalette::Dark);
	m_scrollArea->setWidget(m_imageLabel);
	m_scrollArea->setVisible(true);
	QGridLayout* layout = new QGridLayout();
	layout->addWidget(m_scrollArea, 0, 0);
	layout->rowStretch(0);
	layout->setColumnStretch(0, 100);
	setLayout(layout);
	QSize wndsize = QDesktopWidget().availableGeometry(this).size() * 0.7;
	resize(wndsize);
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    m_imageLabel->adjustSize();
    m_scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = m_fitToWindowAct->isChecked();
    m_scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(m_imageLabel->pixmap());
    m_scaleFactor *= factor;
    m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap()->size());

    adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

    m_zoomInAct->setEnabled(m_scaleFactor < 3.0);
    m_zoomOutAct->setEnabled(m_scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

