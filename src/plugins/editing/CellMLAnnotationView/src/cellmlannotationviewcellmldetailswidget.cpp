//==============================================================================
// CellML annotation view CellML details widget
//==============================================================================

#include "borderedwidget.h"
#include "cellmlannotationviewcellmldetailswidget.h"
#include "cellmlannotationviewcellmlmetadatadetailswidget.h"
#include "cellmlannotationviewmetadatabiomodelsdotnetviewdetailswidget.h"
#include "cellmlannotationviewmetadatarawviewdetailswidget.h"
#include "cellmlannotationviewwidget.h"
#include "cellmlannotationviewmetadataviewdetailswidget.h"

//==============================================================================

#include "ui_cellmlannotationviewcellmldetailswidget.h"

//==============================================================================

#include <QComboBox>
#include <QWebView>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

CellmlAnnotationViewCellmlDetailsWidget::CellmlAnnotationViewCellmlDetailsWidget(CellmlAnnotationViewWidget *pParent) :
    QSplitter(pParent),
    CommonWidget(pParent),
    mParent(pParent),
    mGui(new Ui::CellmlAnnotationViewCellmlDetailsWidget)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create our details widgets

    mCellmlElementDetails  = new CellmlAnnotationViewCellmlElementDetailsWidget(pParent);
    mCellmlMetadataDetails = new CellmlAnnotationViewCellmlMetadataDetailsWidget(pParent);

    // Add our details widgets to our splitter

    addWidget(new Core::BorderedWidget(mCellmlElementDetails,
                                       false, true, true, false));
    addWidget(mCellmlMetadataDetails);

    // Keep track of our splitter being moved

    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(emitSplitterMoved()));
}

//==============================================================================

CellmlAnnotationViewCellmlDetailsWidget::~CellmlAnnotationViewCellmlDetailsWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::retranslateUi()
{
    // Retranslate our GUI
    // Note: we must also update the connection for our cmeta:id widget since it
    //       gets recreated as a result of the retranslation...

    if (mCellmlElementDetails->cmetaIdValue())
        disconnect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                   this, SLOT(newCmetaId(const QString &)));

    mGui->retranslateUi(this);

    mCellmlElementDetails->retranslateUi();
    mCellmlMetadataDetails->retranslateUi();

    if (mCellmlElementDetails->cmetaIdValue())
        connect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                this, SLOT(newCmetaId(const QString &)));
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::updateGui(const CellmlAnnotationViewCellmlElementDetailsWidget::Items &pItems)
{
    static CellmlAnnotationViewCellmlElementDetailsWidget::Items oldItems = CellmlAnnotationViewCellmlElementDetailsWidget::Items();

    if (pItems == oldItems)
        // We want to show the same items, so...

        return;

    // Keep track of the items

    oldItems = pItems;

    // Stop tracking changes to the cmeta:id value of our CellML element

    if (mCellmlElementDetails->cmetaIdValue())
        disconnect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                   this, SLOT(newCmetaId(const QString &)));

    // Update our CellML element details GUI

    mCellmlElementDetails->updateGui(pItems);

    // Re-track changes to the cmeta:id value of our CellML element and update
    // our metadata details GUI

    if (mCellmlElementDetails->cmetaIdValue()) {
        connect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                this, SLOT(newCmetaId(const QString &)));

        newCmetaId(mCellmlElementDetails->cmetaIdValue()->currentText());
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::updateSizes(const QList<int> &pSizes)
{
    // The splitter of another CellmlAnnotationViewCellmlDetailsWidget object
    // has been moved, so update our sizes

    setSizes(pSizes);
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::emitSplitterMoved()
{
    // Let people know that our splitter has been moved

    emit splitterMoved(sizes());
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::newCmetaId(const QString &pCmetaId)
{
    // Retrieve the RDF triples for the cmeta:id

    CellMLSupport::CellmlFileRdfTriples rdfTriples = mParent->cellmlFile()->rdfTriples(pCmetaId);

    // Check that we are not dealing with the same RDF triples
    // Note: this may happen when manually typing the name of a cmeta:id and the
    //       QComboBox suggesting something for you, e.g. you start typing "C_"
    //       and the QComboBox suggests "C_C" (which will get us here) and then
    //       you finish typing "C_C" (which will also get us here)

    static CellMLSupport::CellmlFileRdfTriples oldRdfTriples = CellMLSupport::CellmlFileRdfTriples(mParent->cellmlFile());

    if (rdfTriples == oldRdfTriples)
        return;

    oldRdfTriples = rdfTriples;

    // Update its metadata details

    mCellmlMetadataDetails->metadataViewDetails()->updateGui(rdfTriples);
}

//==============================================================================

void CellmlAnnotationViewCellmlDetailsWidget::metadataUpdated()
{
    // Some metadata has been updated, so we need to update the metadata
    // information we show to the user

    if (mCellmlElementDetails->cmetaIdValue())
        disconnect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                   this, SLOT(newCmetaId(const QString &)));

    QString cmetaIdValue = mCellmlElementDetails->cmetaIdValue()?mCellmlElementDetails->cmetaIdValue()->currentText():QString();

    mCellmlElementDetails->updateGui();

    if (mCellmlElementDetails->cmetaIdValue())
        connect(mCellmlElementDetails->cmetaIdValue(), SIGNAL(editTextChanged(const QString &)),
                this, SLOT(newCmetaId(const QString &)));

    newCmetaId(cmetaIdValue);
}

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
