//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//

#include "MarbleModel.h"

#include <cmath>

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include "GeoPolygon.h"
#include "texcolorizer.h"
#include "TileLoader.h"
#include "tilescissor.h"
#include "katlasdirs.h"
#include "katlastilecreatordialog.h"
#include "placemarkmanager.h"
#include "xmlhandler.h"

const float RAD2INT = 21600.0 / M_PI;


class MarbleModelPrivate
{
 public:
    QWidget  *m_parent;
    QImage   *m_canvasimg;
    QImage   *m_coastimg;

    // View and paint stuff
    MapTheme          *m_maptheme;
    TextureColorizer  *m_texcolorizer;
    TextureMapper     *m_texmapper;
    VectorComposer    *m_veccomposer;
    GridMap           *m_gridmap;

    // Places on the map
    PlaceMarkManager  *m_placemarkmanager;
    PlaceMarkModel    *m_placemarkmodel;
    PlaceMarkPainter  *m_placemarkpainter;
    PlaceContainer    *m_placecontainer;

    Quaternion         m_planetAxis;
    Quaternion         m_planetAxisUpdated;
    int                m_radius;
    int                m_radiusUpdated;

    bool  m_justModified;
    bool  m_centered;

    bool          m_showGrid;
    bool          m_showPlaceMarks;
    bool          m_showElevationModel;
};



MarbleModel::MarbleModel( QWidget* parent )
    : d( new MarbleModelPrivate )
{
    d->m_parent = parent;

    d->m_texmapper = 0;

    d->m_placemarkpainter = 0;
    d->m_placecontainer   = 0;
    d->m_radius           = 2000;

    d->m_justModified = false;

    d->m_showGrid = true;
    d->m_showPlaceMarks = true;
    d->m_showElevationModel = false;

    d->m_planetAxis   = Quaternion( 1.0, 0.0, 0.0, 0.0 );

    d->m_coastimg = new QImage( 10, 10, QImage::Format_ARGB32_Premultiplied );
    d->m_maptheme = new MapTheme();


    QStringList  mapthemedirs = MapTheme::findMapThemes( "maps/earth/" );
    QString      selectedmap;

    if ( mapthemedirs.count() == 0 ) {
        qDebug() << "Couldn't find any maps! Exiting ...";
        exit(-1);
    }

    if ( mapthemedirs.count() >= 1 ) {
        QStringList  tmp = mapthemedirs.filter( "etopo2.dgml" );
        if ( tmp.count() >= 1 )
            selectedmap = tmp[0];
        else
            selectedmap = mapthemedirs[0];
    }
    setMapTheme( selectedmap );

    d->m_veccomposer  = new VectorComposer();
    d->m_gridmap      = new GridMap();
    d->m_texcolorizer = new TextureColorizer( KAtlasDirs::path( "seacolors.leg" ), 
                                           KAtlasDirs::path( "landcolors.leg" ) );

    d->m_placemarkmanager = new PlaceMarkManager();
    d->m_placecontainer   = d->m_placemarkmanager->getPlaceContainer();

    d->m_placemarkmodel   = new PlaceMarkModel( this );
    d->m_placemarkmodel->setContainer( d->m_placecontainer );
}

MarbleModel::~MarbleModel()
{
    delete d->m_texmapper;
    delete d;
}


bool MarbleModel::showGrid() const
{
    return d->m_showGrid;
}

void MarbleModel::setShowGrid( bool visible )
{ 
    d->m_showGrid = visible;
}

bool MarbleModel::showPlaceMarks() const
{ 
    return d->m_showPlaceMarks;
}

void MarbleModel::setShowPlaceMarks( bool visible )
{ 
    d->m_showPlaceMarks = visible;
}

bool MarbleModel::showElevationModel() const
{ 
    return d->m_showElevationModel;
}

void MarbleModel::setShowElevationModel( bool visible )
{ 
    d->m_showElevationModel = visible;
}


// Set a particular theme for the map, and load the top 3 tile levels.
// If these tiles aren't already created, then create them here and now. 

void MarbleModel::setMapTheme( const QString& selectedmap )
{

    d->m_maptheme->open( KAtlasDirs::path( QString("maps/earth/%1")
                                        .arg( selectedmap ) ) );

    // If the tiles aren't already there, put up a progress dialog
    // while creating them.
    if ( !TileLoader::baseTilesAvailable( d->m_maptheme->tilePrefix() ) ) {
        qDebug("Base tiles not available. Creating Tiles ... ");

#if 1
        KAtlasTileCreatorDialog tilecreatordlg( d->m_parent );
        tilecreatordlg.setSummary( d->m_maptheme->name(), 
                                   d->m_maptheme->description() );
#endif

        TileScissor tilecreator( d->m_maptheme->prefix(),
                                 d->m_maptheme->installMap(), 
                                 d->m_maptheme->bitmaplayer().dem);

        // This timer is necessary, because if we remove it, the GUI
        // never gets shown before the work starts.
        QTimer::singleShot( 0, &tilecreator, SLOT( createTiles() ) );
#if 1
        connect( &tilecreator,    SIGNAL( progress( int ) ),
                 &tilecreatordlg, SLOT( setProgress( int ) ) );

        tilecreatordlg.exec();
#else

        connect( &tilecreator, SIGNAL( progress( int ) ),
                 this,         SIGNAL( creatingTilesProgress( int ) ) );
        qDebug("Before emitting creatingTilesStart() ... ");
        emit creatingTilesStart( d->m_maptheme->name(), 
                                 d->m_maptheme->description() );
        qDebug("After emitting creatingTilesStart() ... ");
#endif
    }

    if ( d->m_texmapper == 0 )
        d->m_texmapper = new TextureMapper( "maps/earth/"
                                         + d->m_maptheme->tilePrefix() );
    else
        d->m_texmapper->setMap( "maps/earth/" + d->m_maptheme->tilePrefix() );

    d->m_texmapper->setMaxTileLevel( TileLoader::maxPartialTileLevel( d->m_maptheme->tilePrefix() ) + 1 );

    if ( d->m_placecontainer == 0)
        d->m_placecontainer = new PlaceContainer("placecontainer");

    d->m_placecontainer ->clearTextPixmaps();

    if ( d->m_placemarkpainter == 0)
        d->m_placemarkpainter = new PlaceMarkPainter( this );

    d->m_placemarkpainter->setLabelColor( d->m_maptheme->labelColor() );

    emit themeChanged();

    d->m_justModified = true;
}


void MarbleModel::resize()
{
    *d->m_coastimg = QImage( d->m_canvasimg->width(), d->m_canvasimg->height(),
                          QImage::Format_ARGB32_Premultiplied );
    d->m_canvasimg->fill( Qt::transparent );

    d->m_texmapper->resizeMap( d->m_canvasimg );
    d->m_veccomposer->resizeMap( d->m_coastimg );
    d->m_gridmap->resizeMap( d->m_coastimg );

    QRadialGradient  grad1( QPointF( d->m_canvasimg->width()  / 2,
                                     d->m_canvasimg->height() / 2 ),
                            1.05 * d->m_radius );
    grad1.setColorAt( 0.91, QColor( 255, 255, 255, 255 ) );
    grad1.setColorAt( 1.0,  QColor( 255, 255, 255, 0 ) );

    QBrush    brush1( grad1 );
    QPainter  painter( d->m_canvasimg );
    painter.setBrush( brush1 );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.drawEllipse( d->m_canvasimg->width() / 2 - (int)( (float)(d->m_radius) * 1.05 ),
                         d->m_canvasimg->height() / 2 - (int)( (float)(d->m_radius) * 1.05 ),
                         (int)( 2.1 * (float)(d->m_radius) ), 
                         (int)( 2.1 * (float)(d->m_radius) ) );

    d->m_justModified = true;
}


void MarbleModel::paintGlobe(ClipPainter* painter, const QRect& dirty)
{
    if ( needsUpdate() || d->m_canvasimg->isNull() || d->m_justModified == true ) {

        d->m_texmapper->mapTexture( d->m_canvasimg, d->m_radius, d->m_planetAxis );

        if ( d->m_showElevationModel == false && d->m_maptheme->bitmaplayer().dem == "true" ){
            d->m_coastimg->fill(Qt::transparent);

            // Create VectorMap
            d->m_veccomposer->drawTextureMap( d->m_coastimg, d->m_radius, 
                                           d->m_planetAxis );

            // Recolorize the heightmap using the VectorMap
            d->m_texcolorizer->colorize( d->m_canvasimg, d->m_coastimg, d->m_radius );
        }
    }

    // Paint Map on Widget
    painter->drawImage( dirty, *d->m_canvasimg, dirty ); 

    if ( d->m_maptheme->vectorlayer().enabled == true ) {

        // Add further Vectors
        d->m_veccomposer->paintVectorMap( painter, d->m_radius, d->m_planetAxis );
    }

    // if ( d->m_maptheme->vectorlayer().enabled == true ){
    QPen  gridpen( QColor( 255, 255, 255, 128 ) );

    if ( d->m_showGrid == true )
    {
        d->m_gridmap->createGrid( d->m_radius, d->m_planetAxis );

        d->m_gridmap->setPen( gridpen );
        d->m_gridmap->paintGridMap( painter, true );

        d->m_gridmap->createTropics( d->m_radius, d->m_planetAxis );

        gridpen.setStyle( Qt::DotLine );
        d->m_gridmap->setPen( gridpen );
        d->m_gridmap->paintGridMap( painter, true );
    }

    //	}
	
    if ( d->m_showPlaceMarks == true && d->m_placecontainer->size() > 0 ) {
        d->m_placemarkpainter->paintPlaceFolder( painter, 
                                              d->m_canvasimg->width() / 2,
                                              d->m_canvasimg->height()/ 2,
                                              d->m_radius, d->m_placecontainer,
                                              d->m_planetAxis );
    }

    d->m_planetAxisUpdated = d->m_planetAxis;
    d->m_radiusUpdated     = d->m_radius;
    d->m_justModified      = false;
}


void MarbleModel::setCanvasImage(QImage* canvasimg)
{
    d->m_canvasimg = canvasimg;
}


int MarbleModel::radius() const
{
    return d->m_radius; 
}

void MarbleModel::setRadius(const int& radius)
{
    // Clear canvas if the globe is visible as a whole or if the globe
    // does shrink.
    int  imgrx = ( d->m_canvasimg->width() ) >> 1;
    int  imgry = ( d->m_canvasimg->height() ) >> 1;

    if ( radius * radius < imgrx * imgrx + imgry * imgry
         && radius != d->m_radius )
    {
        d->m_canvasimg->fill( Qt::transparent );

        QRadialGradient grad1( QPointF( d->m_canvasimg->width() / 2,
                                        d->m_canvasimg->height() / 2 ),
                               1.05 * radius );
        grad1.setColorAt( 0.91, QColor( 255, 255, 255, 255 ) );
        grad1.setColorAt( 1.0,  QColor( 255, 255, 255, 0 ) );
        QBrush    brush1( grad1 );
        QPainter  painter( d->m_canvasimg );
        painter.setBrush( brush1 );
        painter.setRenderHint( QPainter::Antialiasing, true );
        painter.drawEllipse( d->m_canvasimg->width() / 2 - (int)( (float)(radius) * 1.05 ),
                             d->m_canvasimg->height() / 2 - (int)( (float)(radius) * 1.05 ),
                             (int)( 2.1 * (float)(radius) ), 
                             (int)( 2.1 * (float)(radius) ) );
    }

    d->m_radius = radius;
}


Quaternion MarbleModel::getPlanetAxis() const
{
    return d->m_planetAxis;
}


void MarbleModel::rotateTo(const uint& phi, const uint& theta, const uint& psi)
{
    d->m_planetAxis.createFromEuler( (float)(phi)   / RAD2INT,
                                  (float)(theta) / RAD2INT,
                                  (float)(psi)   / RAD2INT );
}

void MarbleModel::rotateTo(const float& phi, const float& theta)
{
    d->m_planetAxis.createFromEuler( (phi + 180.0) * M_PI / 180.0,
                                     (theta + 180.0) * M_PI / 180.0, 0.0 );
}


void MarbleModel::rotateBy(const Quaternion& incRot)
{
    d->m_planetAxis = incRot * d->m_planetAxis;
}

void MarbleModel::rotateBy(const float& phi, const float& theta)
{
    Quaternion  rotPhi( 1.0, phi, 0.0, 0.0 );
    Quaternion  rotTheta( 1.0, 0.0, theta, 0.0 );

    d->m_planetAxis = rotTheta * d->m_planetAxis;
    d->m_planetAxis *= rotPhi;
    d->m_planetAxis.normalize();
}

float MarbleModel::centerLatitude() const
{ 
    return d->m_planetAxis.pitch() * 180.0f / M_PI;
}

float MarbleModel::centerLongitude() const
{
    return - d->m_planetAxis.yaw() * 180.0f / M_PI;
}


QAbstractListModel *MarbleModel::getPlaceMarkModel() const
{
    return d->m_placemarkmodel;
}


bool MarbleModel::needsUpdate() const
{
    return !( d->m_radius == d->m_radiusUpdated
              && d->m_planetAxis == d->m_planetAxisUpdated ); 
}

void MarbleModel::setNeedsUpdate()
{
    d->m_justModified = true;
}


int MarbleModel::northPoleY()
{
    
    Quaternion  northPole   = GeoPoint( 0.0f, (float)( -M_PI * 0.5 ) ).quaternion();
    Quaternion  invPlanetAxis = d->m_planetAxis.inverse();

    northPole.rotateAroundAxis(invPlanetAxis);

    return (int)( d->m_radius * northPole.v[Q_Y] );
}

int MarbleModel::northPoleZ()
{
    Quaternion  northPole   = GeoPoint( 0.0f, (float)( -M_PI * 0.5 ) ).quaternion();
    Quaternion  invPlanetAxis = d->m_planetAxis.inverse();

    northPole.rotateAroundAxis(invPlanetAxis);

    return (int)( d->m_radius * northPole.v[Q_Z] );
}

PlaceContainer    *MarbleModel::placeContainer()   const
{
    return d->m_placecontainer;
}

VectorComposer    *MarbleModel::vectorComposer()   const
{
    return d->m_veccomposer;
}

TextureColorizer  *MarbleModel::textureColorizer() const
{
    return d->m_texcolorizer;
}

TextureMapper     *MarbleModel::textureMapper()    const
{
    return d->m_texmapper;
}

PlaceMarkPainter  *MarbleModel::placeMarkPainter() const 
{
    return d->m_placemarkpainter;
}

bool MarbleModel::screenCoordinates( const float lng, const float lat, 
                                     int& x, int& y )
{
    Quaternion  qpos       = GeoPoint( lng, lat ).quaternion();
    Quaternion  invRotAxis = d->m_planetAxis.inverse();

    qpos.rotateAroundAxis(invRotAxis);

    x = (int)(  d->m_radius * qpos.v[Q_X] );
    y = (int)( -d->m_radius * qpos.v[Q_Y] );

    if ( qpos.v[Q_Z] >= 0.0 )
        return true;
    else 
        return false;
}

void MarbleModel::addPlaceMarkFile( const QString& filename )
{
    d->m_placemarkmanager->loadKml( filename );

    d->m_placecontainer = d->m_placemarkmanager->getPlaceContainer();

    d->m_placemarkmodel->setContainer( d->m_placecontainer );	
}

QVector< PlaceMark* > MarbleModel::whichFeatureAt( const QPoint& curpos )
{
    return d->m_placemarkpainter->whichPlaceMarkAt( curpos );
}

#include "MarbleModel.moc"
