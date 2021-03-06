#include "importmidi_opmodel.h"
#include "importmidi_operations.h"


namespace Ms {

struct Node {
      QString name;
      Operation oper;
      QStringList values;
      bool visible = true;
      Node *parent = nullptr;
      std::vector<std::unique_ptr<Node> > children;
      };

struct Controller {
      Node *LHRHdoIt = nullptr;
      Node *LHRHMethod = nullptr;
      Node *LHRHPitchOctave = nullptr;
      Node *LHRHPitchNote = nullptr;
      Node *quantValue = nullptr;
      Node *quantReduce = nullptr;

      bool updateNodesDependencies(Node *node, bool force_update);
      };

OperationsModel::OperationsModel()
            : root(std::unique_ptr<Node>(new Node()))
            , controller(std::unique_ptr<Controller>(new Controller()))
      {
      beginResetModel();

      // - initialize opeations with their default values
      // - string lists below should match Operation enum values

      Node *quantValue = new Node;
      quantValue->name = "Quantization";
      quantValue->oper.type = Operation::QUANT_VALUE;
      quantValue->oper.value = TrackOperations().quantize.value;
      quantValue->values.push_back("Shortest note in bar");
      quantValue->values.push_back("Value from preferences");
      quantValue->values.push_back("1/4");
//    quantValue->values.push_back("1/4 triplet");
      quantValue->values.push_back("1/8");
//    quantValue->values.push_back("1/8 triplet");
      quantValue->values.push_back("1/16");
//    quantValue->values.push_back("1/16 triplet");
      quantValue->values.push_back("1/32");
//    quantValue->values.push_back("1/32 triplet");
      quantValue->values.push_back("1/64");
      quantValue->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(quantValue));
      controller->quantValue = quantValue;


      Node *reduceToShorter = new Node;
      reduceToShorter->name = "Reduce to shorter notes in bar";
      reduceToShorter->oper.type = Operation::QUANT_REDUCE;
      reduceToShorter->oper.value = Quantization().reduceToShorterNotesInBar;
      reduceToShorter->parent = quantValue;
      quantValue->children.push_back(std::unique_ptr<Node>(reduceToShorter));
      controller->quantReduce = reduceToShorter;


      Node *useDots = new Node;
      useDots->name = "Use dots";
      useDots->oper.type = Operation::USE_DOTS;
      useDots->oper.value = TrackOperations().useDots;
      useDots->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(useDots));


      Node *doLHRH = new Node;
      doLHRH->name = "LH/RH separation";
      doLHRH->oper.type = Operation::DO_LHRH_SEPARATION;
      doLHRH->oper.value = LHRHSeparation().doIt;
      doLHRH->parent = root.get();
      root->children.push_back(std::unique_ptr<Node>(doLHRH));
      controller->LHRHdoIt = doLHRH;


      Node *LHRHMethod = new Node;
      LHRHMethod->name = "Separation method";
      LHRHMethod->oper.type = Operation::LHRH_METHOD;
      LHRHMethod->oper.value = LHRHSeparation().method;
      LHRHMethod->values.push_back("Hand width");
      LHRHMethod->values.push_back("Fixed pitch");
      LHRHMethod->parent = doLHRH;
      doLHRH->children.push_back(std::unique_ptr<Node>(LHRHMethod));
      controller->LHRHMethod = LHRHMethod;


      Node *LHRHPitchOctave = new Node;
      LHRHPitchOctave->name = "Split pitch octave";
      LHRHPitchOctave->oper.type = Operation::LHRH_SPLIT_OCTAVE;
      LHRHPitchOctave->oper.value = LHRHSeparation().splitPitchOctave;
      LHRHPitchOctave->values.push_back("C-1");
      LHRHPitchOctave->values.push_back("C0");
      LHRHPitchOctave->values.push_back("C1");
      LHRHPitchOctave->values.push_back("C2");
      LHRHPitchOctave->values.push_back("C3");
      LHRHPitchOctave->values.push_back("C4");
      LHRHPitchOctave->values.push_back("C5");
      LHRHPitchOctave->values.push_back("C6");
      LHRHPitchOctave->values.push_back("C7");
      LHRHPitchOctave->values.push_back("C8");
      LHRHPitchOctave->values.push_back("C9");
      LHRHPitchOctave->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchOctave));
      controller->LHRHPitchOctave = LHRHPitchOctave;


      Node *LHRHPitchNote = new Node;
      LHRHPitchNote->name = "Split pitch note";
      LHRHPitchNote->oper.type = Operation::LHRH_SPLIT_NOTE;
      LHRHPitchNote->oper.value = LHRHSeparation().splitPitchNote;
      LHRHPitchNote->values.push_back("C"); 
      LHRHPitchNote->values.push_back("C#");
      LHRHPitchNote->values.push_back("D"); 
      LHRHPitchNote->values.push_back("D#");
      LHRHPitchNote->values.push_back("E"); 
      LHRHPitchNote->values.push_back("F"); 
      LHRHPitchNote->values.push_back("F#");
      LHRHPitchNote->values.push_back("G"); 
      LHRHPitchNote->values.push_back("G#");
      LHRHPitchNote->values.push_back("A"); 
      LHRHPitchNote->values.push_back("A#");
      LHRHPitchNote->values.push_back("H");
      LHRHPitchNote->parent = LHRHMethod;
      LHRHMethod->children.push_back(std::unique_ptr<Node>(LHRHPitchNote));
      controller->LHRHPitchNote = LHRHPitchNote;

      //--------------------------------------------------------------------
      connect(this,
              SIGNAL(dataChanged(QModelIndex,QModelIndex)),
              SLOT(onDataChanged(QModelIndex)));
      controller->updateNodesDependencies(nullptr, true);
      endResetModel();
      }

OperationsModel::~OperationsModel()
      {
      }

QModelIndex OperationsModel::index(int row, int column, const QModelIndex &parent) const
      {
      if (!root || row < 0 || column < 0 || column >= OperationCol::COL_COUNT)
            return QModelIndex();
      Node *parent_node = nodeFromIndex(parent);
      if (!parent_node)
            return QModelIndex();
      if (parent_node->children.empty() || row >= (int)parent_node->children.size())
            return QModelIndex();
      // find new row in connection with invisible items
      int shift = 0;
      for (int i = 0; i <= row + shift; ++i) {
            if (i >= (int)parent_node->children.size())
                  return QModelIndex();
            if (!parent_node->children.at(i)->visible)
                  ++shift;
            }
      Node *child_node = parent_node->children.at(row + shift).get();
      if (!child_node || !child_node->visible)
            return QModelIndex();
      return createIndex(row, column, child_node);
      }

QModelIndex OperationsModel::parent(const QModelIndex &child) const
      {
      Node *node = nodeFromIndex(child);
      if (!node)
          return QModelIndex();
      Node *parent_node = node->parent;
      if (!parent_node)
          return QModelIndex();
      Node *grandparent_node = parent_node->parent;
      if (!grandparent_node)
          return QModelIndex();
      auto &v = grandparent_node->children;
      auto iter = std::find_if(v.begin(), v.end(),
          [parent_node](std::unique_ptr<Node> &el){ return el.get() == parent_node; });
      int row = (iter == v.end()) ? -1 : iter - v.begin();
      return createIndex(row, 0, parent_node);
      }

int OperationsModel::rowCount(const QModelIndex &parent) const
      {
      if (parent.column() >= OperationCol::COL_COUNT)
            return 0;
      Node *parent_node = nodeFromIndex(parent);
      if (!parent_node)
            return 0;
      // take only visible nodes into account
      size_t counter = 0;
      for (const auto &p: parent_node->children)
            if (p->visible)
                  ++counter;
      return counter;
      }

int OperationsModel::columnCount(const QModelIndex &parent) const
      {
      return OperationCol::COL_COUNT;
      }

// All nodes can have either bool value or list of possible values
// also node value can be undefined (QVariant()), for example grayed checkbox
QVariant OperationsModel::data(const QModelIndex &index, int role) const
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return QVariant();
      switch (role) {
            case DataRole:
                  if (node->values.empty())  // checkbox
                        return node->oper.value.toBool();
                  else
                        return node->oper.value.toInt();
                  break;
            case Qt::DisplayRole:
                  switch (index.column()) {
                        case OperationCol::OPER_NAME:
                              return node->name;
                        case OperationCol::VALUE:
                              if (!node->values.empty()) {
                                    if (!node->oper.value.isValid()) // undefined operation value
                                          return " . . . ";
                                    // list contains names of possible string values
                                    // like {"1/4", "1/8"}
                                    // valid node value is one of enum items
                                    // -> use enum item as index
                                    int indexOfValue = node->oper.value.toInt();
                                    if (indexOfValue < node->values.size() && indexOfValue >= 0)
                                          return node->values.at(indexOfValue);
                                    }
                              // otherwise return nothing because it's a checkbox
                              break;
                        default:
                              break;
                        }
                  break;
            case Qt::EditRole:
                  if (index.column() == OperationCol::VALUE && !node->values.empty())
                        return node->values;
                  break;
            case Qt::CheckStateRole:
                  if (index.column() == OperationCol::VALUE && node->values.empty()) {
                        if (!node->oper.value.isValid())
                              return Qt::PartiallyChecked;
                        return (node->oper.value.toBool())
                               ? Qt::Checked : Qt::Unchecked;
                        }
                  break;
            case Qt::SizeHintRole:
                  {
                  QSize sz;
                  sz.setHeight(22);
                  return sz;
                  }
            case OperationTypeRole:
                  return node->oper.type;
            default:
                  break;
            }
      return QVariant();
      }

QVariant OperationsModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            {
            switch (section) {
                  case OperationCol::OPER_NAME:
                        return "Selected track [" + trackLabel + "] operations";
                  case OperationCol::VALUE:
                        return "Value";
                  default:
                        break;
                  }
            }
      return QVariant();
      }

Qt::ItemFlags OperationsModel::flags(const QModelIndex &index) const
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return Qt::ItemFlags();
      Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled);
      if (index.column() == OperationCol::VALUE) {
            if (node->values.empty()) // node value is bool - a checkbox
                  flags |= Qt::ItemIsUserCheckable;
            else // node has list of values
                  flags |= Qt::ItemIsEditable;
            }
      return flags;
      }

bool OperationsModel::setData(const QModelIndex &index, const QVariant &value, int role)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return false;
      bool result = false;
      if (index.column() == OperationCol::VALUE) {
            switch (role) {
                  case Qt::CheckStateRole:
                        node->oper.value = value.toBool();
                        result = true;
                        break;
                  case Qt::EditRole:
                        // set enum value from value == list index
                        node->oper.value = value.toInt();
                        result = true;
                        break;
                  default:
                        break;
                  }
            }
      if (result)
            emit dataChanged(index, index);
      return result;
      }

void setNodeOperations(Node *node, const DefinedTrackOperations &opers)
      {
      if (opers.undefinedOpers.contains(node->oper.type))
            node->oper.value = QVariant();
      else {
            switch (node->oper.type) {
                  case Operation::QUANT_VALUE:
                        node->oper.value = opers.opers.quantize.value; break;
                  case Operation::QUANT_REDUCE:
                        node->oper.value = opers.opers.quantize.reduceToShorterNotesInBar; break;
        
                  case Operation::DO_LHRH_SEPARATION:
                        node->oper.value = opers.opers.LHRH.doIt; break;
                  case Operation::LHRH_METHOD:
                        node->oper.value = opers.opers.LHRH.method; break;
                  case Operation::LHRH_SPLIT_OCTAVE:
                        node->oper.value = opers.opers.LHRH.splitPitchOctave; break;
                  case Operation::LHRH_SPLIT_NOTE:
                        node->oper.value = opers.opers.LHRH.splitPitchNote; break;
                  case Operation::USE_DOTS:
                        node->oper.value = opers.opers.useDots; break;
                  case Operation::DO_IMPORT: break;
                  }
            }
      for (const auto &nodePtr: node->children)
            setNodeOperations(nodePtr.get(), opers);
      }

void OperationsModel::setTrackData(const QString &trackLabel, const DefinedTrackOperations &opers)
      {
      this->trackLabel = trackLabel;
      // set new operations values
      beginResetModel();
      for (const auto &nodePtr: root->children)
            setNodeOperations(nodePtr.get(), opers);
      controller->updateNodesDependencies(nullptr, true);
      endResetModel();
      }

void OperationsModel::onDataChanged(const QModelIndex &index)
      {
      Node *node = nodeFromIndex(index);
      if (!node)
            return;
      if (controller->updateNodesDependencies(node, false))
            layoutChanged();
      }

Node* OperationsModel::nodeFromIndex(const QModelIndex &index) const
      {
      if (index.isValid())
            return static_cast<Node *>(index.internalPointer());
      else
            return root.get();
      }

// Different controller actions, i.e. conditional visibility of node

bool Controller::updateNodesDependencies(Node *node, bool force_update)
      {
      bool result = false;
      if (!node && !force_update)
            return result;
      if (force_update || (LHRHMethod && node == LHRHMethod)) {
            auto value = (Operation::LHRHMethod)LHRHMethod->oper.value.toInt();
            switch (value) {
                  case Operation::HAND_WIDTH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = false;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = false;
                        result = true;
                        break;
                  case Operation::SPECIFIED_PITCH:
                        if (LHRHPitchOctave)
                              LHRHPitchOctave->visible = true;
                        if (LHRHPitchNote)
                              LHRHPitchNote->visible = true;
                        result = true;
                        break;
                  }
            }
      if (force_update || (LHRHdoIt && node == LHRHdoIt)) {
            auto value = LHRHdoIt->oper.value.toBool();
            if (LHRHMethod)
                  LHRHMethod->visible = value;
            result = true;
            }
      if (force_update || (quantValue && node == quantValue)) {
            auto value = (Operation::QuantValue)quantValue->oper.value.toInt();
            if (quantReduce)
                  quantReduce->visible = (value != Operation::SHORTEST_IN_BAR);
            result = true;
            }
      // other nodes similar, if any
      //
      return result;
      }

} // namespace Ms
