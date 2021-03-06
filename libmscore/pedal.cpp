//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "pedal.h"
#include "textline.h"
#include "sym.h"

#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PedalSegment::layout()
      {
      rypos() = 0.0;
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(ST_pedalY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLine(s)
      {
      setBeginSymbol(pedalPedSym);
      setBeginSymbolOffset(QPointF(0.0, -.2));

      setEndHook(true);
      setBeginHookHeight(Spatium(-1.2));
      setEndHookHeight(Spatium(-1.2));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(XmlReader& e)
      {
      if (score()->mscVersion() >= 110) {
            setBeginSymbol(noSym);
            setEndHook(false);
            }
      if (e.name() == "subtype")          // obsolete
            e.skipCurrentElement();
      TextLine::read(e);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Pedal::createLineSegment()
      {
      return new PedalSegment(score());
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Pedal::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_pedalY).val()) * spatium();
      }

}

