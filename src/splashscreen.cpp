#include "splashscreen.h"

namespace ServerControlPanel
{

    SplashScreen::SplashScreen(const QPixmap &pixmap, Qt::WindowFlags f) :
        QSplashScreen(pixmap, f)
    {
        this->setAutoFillBackground(true);
        this->setCursor(Qt::BusyCursor);

        // set reference point, paddings
        int paddingRight            = 20;
        int paddingTop              = 50;
        int titleVersionVSpace      = 17;
        int titleCopyrightVSpace    = 35;

        double physicalDPI = qApp->primaryScreen()->physicalDotsPerInch();
        double scaleFactor = QString::number((physicalDPI / 160), 'f', 2 ).toDouble(); // scale factor normalized to 160 DPI
        float fontFactor   = (((15 * 0.03937) * scaleFactor) + 0.5);

        // define texts
        QString titleText       = QString(QApplication::applicationName());
        QString versionText     = QString("Version %1").arg(QString::fromStdString(APP_VERSION_SHORT));
        QString copyrightText   = QChar(0xA9)+QString(" 2010-%1 ").arg(QDate::currentDate().toString("yyyy")) + QString(tr("Jens-André Koch"));
        //QString madeinText      = QString("Made in Germany.");

        QString font            = "Arial";

        // load bitmap for writing text over it
        QPixmap newPixmap;
        newPixmap = QPixmap(":/splash");

        QPainter pixPaint(&newPixmap);
        pixPaint.setPen(QColor(100,100,100));

        // check font size and drawing width
        pixPaint.setFont(QFont(font, 33*fontFactor));
        QFontMetrics fm = pixPaint.fontMetrics();
        int titleTextWidth  = fm.width(titleText);
        if(titleTextWidth > 160) {
            // strange font rendering, Arial probably not found
            fontFactor = 0.75;
        }

        // title
        pixPaint.setFont(QFont(font, 33*fontFactor));
        titleTextWidth = pixPaint.fontMetrics().width(titleText);
        pixPaint.drawText(newPixmap.width()-titleTextWidth-paddingRight,paddingTop,titleText);

        // version
        pixPaint.setFont(QFont(font, 15*fontFactor));
        int versionTextWidth = pixPaint.fontMetrics().width(versionText);
        pixPaint.drawText(newPixmap.width()-versionTextWidth-paddingRight,paddingTop+titleVersionVSpace,versionText);

        // copyright
        pixPaint.setFont(QFont(font, 10*fontFactor));
        int copyrightTextWidth = pixPaint.fontMetrics().width(copyrightText);
        pixPaint.drawText(newPixmap.width()-copyrightTextWidth-paddingRight-2,paddingTop+titleCopyrightVSpace,copyrightText);

        // made in germany
        //int madeinTextWidth = pixPaint.fontMetrics().width(madeinText) + 5;
        //pixPaint.drawText(newPixmap.width()-madeinTextWidth,newPixmap.height()-5,madeinText);

        pixPaint.end();

        this->setPixmap(newPixmap);

        this->showMessage("Loading...", Qt::AlignBottom | Qt::AlignLeft, Qt::gray);
    }

    void SplashScreen::drawContents(QPainter *painter)
    {
      QSplashScreen::drawContents(painter);

      QString style = "QProgressBar::chunk { background-color: #BBB; margin: 0.5px; }";
      style.append("QProgressBar { border: 1px solid #CCC; background: #EEE;}");

      QProgressBar bar;
      bar.setStyleSheet(style);

      // Set base style for progressbar...
      QStyleOptionProgressBar progressBar;
      progressBar.initFrom(&bar);
      progressBar.state = QStyle::State_Enabled | QStyle::State_Active;
      progressBar.textVisible = false;
      progressBar.minimum = 0;
      progressBar.maximum = 100;
      progressBar.progress = progress;
      progressBar.invertedAppearance = false;
      progressBar.rect = QRect(60, 308, 400, 6); // Position of the progressBar

      painter->save();
      QStyle *drawStyle = bar.style();
      // draw the progress bar onto the view
      drawStyle->drawControl(QStyle::CE_ProgressBar, &progressBar, painter, &bar);
      painter->restore();
    }

    void SplashScreen::setProgress(int value)
    {
      progress = value;
      if (progress > 100) progress = 100;
      if (progress < 0) progress = 0;
      update();
    }
}
