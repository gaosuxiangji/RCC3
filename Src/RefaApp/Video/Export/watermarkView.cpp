/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "watermarkView.h"

#include <QtWidgets>
#include <qmath.h>

#ifndef QT_NO_WHEELEVENT
void GraphicsView::wheelEvent(QWheelEvent *e)
{
   // if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0)
            view->zoomIn(6);
        else
            view->zoomOut(6);
        e->accept();
//     } else {
//         QGraphicsView::wheelEvent(e);
//     }
}
#endif

watermarkView::watermarkView(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(Sunken | StyledPanel);
    graphicsView = new GraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing, false);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(graphicsView, 1);

    setLayout(topLayout);
	connect(this, SIGNAL(signalUpdateZoom()), this, SLOT(slotUpdateZoom()));

    setupMatrix();
}

QGraphicsView *watermarkView::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
}

void watermarkView::setupMatrix()
{
    qreal scale = qPow(qreal(2), (m_nZoomValue - 250) / qreal(50));

    QMatrix matrix;
    matrix.scale(scale, scale);

    graphicsView->setMatrix(matrix);
}

void watermarkView::zoomIn(int level)
{
	m_nZoomValue += level;
	if (m_nZoomValue > constZOOMMAX)
	{
		m_nZoomValue = constZOOMMAX;
	}
	emit signalUpdateZoom();
}

void watermarkView::zoomOut(int level)
{
	m_nZoomValue -= level;
	if (m_nZoomValue < constZOOMMIN)
	{
		m_nZoomValue = constZOOMMIN;
	}
    emit signalUpdateZoom();
}

void watermarkView::AutoAdjust()
{
	graphicsView->fitInView(graphicsView->sceneRect(), Qt::KeepAspectRatio);

	QMatrix matrix = graphicsView->matrix();
	qreal scale = matrix.m11();
	if (scale > 0)
	{
		int oldZoom = 250 + 50*(qLn(scale) / qLn(2));
		if (oldZoom > constZOOMMAX)
		{
			oldZoom = constZOOMMAX;
		}
		else if (oldZoom < constZOOMMIN)
		{
			oldZoom = constZOOMMIN;
		}
		m_nZoomValue = oldZoom;
	}
}

void watermarkView::slotUpdateZoom()
{
	setupMatrix();
}
