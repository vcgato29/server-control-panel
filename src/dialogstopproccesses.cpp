#include "dialogstopproccesses.h"
#include "ui_dialogstopproccesses.h"

DialogStopProccesses::DialogStopProccesses(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogStopProccesses)
{
    ui->setupUi(this);

    QLabel *lblName = new QLabel(TRANSLATE(str::sUser));
    m_leName = new QLineEdit;

    QLabel *lblPass = new QLabel(TRANSLATE(str::sPassword));
    m_lePassword = new QLineEdit;
    m_lePassword->setEchoMode(QLineEdit::Password);

    // Watch settings widgets.
    m_chkSave = new QCheckBox(TRANSLATE(str::sRememberPassword));

    // Create grid layout and add all widgets
    QGridLayout *grid = new QGridLayout;

    grid->addWidget(lblName, 0, 0);
    grid->addWidget(m_leName, 0, 1);
    grid->addWidget(lblPass, 1, 0);
    grid->addWidget(m_lePassword, 1, 1);
    grid->addWidget(m_chkSave, 2, 1);

    // Dialog buttons
    QPushButton *btnOk = new QPushButton(TRANSLATE(str::sOk));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *btnCancel = new QPushButton(TRANSLATE(str::sCancel));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch(1);

    setMinimumWidth(450);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    setLayout(mainLayout);

    setWindowTitle(TRANSLATE(str::sLoginTitle));
}

DialogStopProccesses::~DialogStopProccesses()
{
    delete ui;
}
