#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QtWidgets/QDialog>
#include <QtGui/QImage>

class QAction;
class QLabel;
class QScrollArea;
class QScrollBar;

class ImageViewer : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewer(const QString &);
private slots:
	void zoomIn();
	void zoomOut();
	void normalSize();
	void fitToWindow();

private:
	void scaleImage(double factor);
	void adjustScrollBar(QScrollBar *scrollBar, double factor);

	QImage m_image;
	QLabel *m_imageLabel = nullptr;
	QScrollArea *m_scrollArea = nullptr;
	double m_scaleFactor = 1;
	QAction *m_zoomInAct = nullptr;
	QAction *m_zoomOutAct = nullptr;
	QAction *m_normalSizeAct = nullptr;
	QAction *m_fitToWindowAct = nullptr;
};

#endif
