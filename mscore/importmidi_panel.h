#ifndef IMPORTMIDI_PANEL_H
#define IMPORTMIDI_PANEL_H


namespace Ui {
      class ImportMidiPanel;
      }

class QModelIndex;


namespace Ms {

class TracksModel;
class OperationsModel;
class OperationsDelegate;

class ImportMidiPanel : public QWidget
      {
      Q_OBJECT

   public:
      explicit ImportMidiPanel(QWidget *parent = 0);
      ~ImportMidiPanel();
      static bool isMidiFile(const QString &file);
      void setMidiFile(const QString &file);

   private slots:
      void updateUiOnTimer();
      void onCurrentTrackChanged(const QModelIndex &currentIndex);
      void onOperationChanged(const QModelIndex &index);
      void importMidi();
      void hidePanel();

   private:
      void tweakUi();
      void updateUi();
      bool canImportMidi() const;

      Ui::ImportMidiPanel *ui;
      QTimer *updateUiTimer;
      QString midiFile;
      bool isMidiFileExists;
      TracksModel *tracksModel;
      OperationsModel *operationsModel;
      OperationsDelegate *operationsDelegate;
      };

} // namespace Ms


#endif // IMPORTMIDI_PANEL_H
