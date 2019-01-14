// ***************************************************************************
//
// Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-442911
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ***************************************************************************

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: PersistentParticlesAttributes
//
// Purpose:
//    This class contains attributes for the PersistentParticles operator.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class PersistentParticlesAttributes extends AttributeSubject implements Plugin
{
    private static int PersistentParticlesAttributes_numAdditionalAtts = 11;

    // Enum values
    public final static int PATHTYPEENUM_ABSOLUTE = 0;
    public final static int PATHTYPEENUM_RELATIVE = 1;


    public PersistentParticlesAttributes()
    {
        super(PersistentParticlesAttributes_numAdditionalAtts);

        startIndex = 0;
        stopIndex = 1;
        stride = 1;
        startPathType = PATHTYPEENUM_ABSOLUTE;
        stopPathType = PATHTYPEENUM_ABSOLUTE;
        traceVariableX = new String("");
        traceVariableY = new String("");
        traceVariableZ = new String("");
        connectParticles = false;
        showPoints = false;
        indexVariable = new String("");
    }

    public PersistentParticlesAttributes(int nMoreFields)
    {
        super(PersistentParticlesAttributes_numAdditionalAtts + nMoreFields);

        startIndex = 0;
        stopIndex = 1;
        stride = 1;
        startPathType = PATHTYPEENUM_ABSOLUTE;
        stopPathType = PATHTYPEENUM_ABSOLUTE;
        traceVariableX = new String("");
        traceVariableY = new String("");
        traceVariableZ = new String("");
        connectParticles = false;
        showPoints = false;
        indexVariable = new String("");
    }

    public PersistentParticlesAttributes(PersistentParticlesAttributes obj)
    {
        super(obj);

        startIndex = obj.startIndex;
        stopIndex = obj.stopIndex;
        stride = obj.stride;
        startPathType = obj.startPathType;
        stopPathType = obj.stopPathType;
        traceVariableX = new String(obj.traceVariableX);
        traceVariableY = new String(obj.traceVariableY);
        traceVariableZ = new String(obj.traceVariableZ);
        connectParticles = obj.connectParticles;
        showPoints = obj.showPoints;
        indexVariable = new String(obj.indexVariable);

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return PersistentParticlesAttributes_numAdditionalAtts;
    }

    public boolean equals(PersistentParticlesAttributes obj)
    {
        // Create the return value
        return ((startIndex == obj.startIndex) &&
                (stopIndex == obj.stopIndex) &&
                (stride == obj.stride) &&
                (startPathType == obj.startPathType) &&
                (stopPathType == obj.stopPathType) &&
                (traceVariableX.equals(obj.traceVariableX)) &&
                (traceVariableY.equals(obj.traceVariableY)) &&
                (traceVariableZ.equals(obj.traceVariableZ)) &&
                (connectParticles == obj.connectParticles) &&
                (showPoints == obj.showPoints) &&
                (indexVariable.equals(obj.indexVariable)));
    }

    public String GetName() { return "PersistentParticles"; }
    public String GetVersion() { return "2.0"; }

    // Property setting methods
    public void SetStartIndex(int startIndex_)
    {
        startIndex = startIndex_;
        Select(0);
    }

    public void SetStopIndex(int stopIndex_)
    {
        stopIndex = stopIndex_;
        Select(1);
    }

    public void SetStride(int stride_)
    {
        stride = stride_;
        Select(2);
    }

    public void SetStartPathType(int startPathType_)
    {
        startPathType = startPathType_;
        Select(3);
    }

    public void SetStopPathType(int stopPathType_)
    {
        stopPathType = stopPathType_;
        Select(4);
    }

    public void SetTraceVariableX(String traceVariableX_)
    {
        traceVariableX = traceVariableX_;
        Select(5);
    }

    public void SetTraceVariableY(String traceVariableY_)
    {
        traceVariableY = traceVariableY_;
        Select(6);
    }

    public void SetTraceVariableZ(String traceVariableZ_)
    {
        traceVariableZ = traceVariableZ_;
        Select(7);
    }

    public void SetConnectParticles(boolean connectParticles_)
    {
        connectParticles = connectParticles_;
        Select(8);
    }

    public void SetShowPoints(boolean showPoints_)
    {
        showPoints = showPoints_;
        Select(9);
    }

    public void SetIndexVariable(String indexVariable_)
    {
        indexVariable = indexVariable_;
        Select(10);
    }

    // Property getting methods
    public int     GetStartIndex() { return startIndex; }
    public int     GetStopIndex() { return stopIndex; }
    public int     GetStride() { return stride; }
    public int     GetStartPathType() { return startPathType; }
    public int     GetStopPathType() { return stopPathType; }
    public String  GetTraceVariableX() { return traceVariableX; }
    public String  GetTraceVariableY() { return traceVariableY; }
    public String  GetTraceVariableZ() { return traceVariableZ; }
    public boolean GetConnectParticles() { return connectParticles; }
    public boolean GetShowPoints() { return showPoints; }
    public String  GetIndexVariable() { return indexVariable; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(startIndex);
        if(WriteSelect(1, buf))
            buf.WriteInt(stopIndex);
        if(WriteSelect(2, buf))
            buf.WriteInt(stride);
        if(WriteSelect(3, buf))
            buf.WriteInt(startPathType);
        if(WriteSelect(4, buf))
            buf.WriteInt(stopPathType);
        if(WriteSelect(5, buf))
            buf.WriteString(traceVariableX);
        if(WriteSelect(6, buf))
            buf.WriteString(traceVariableY);
        if(WriteSelect(7, buf))
            buf.WriteString(traceVariableZ);
        if(WriteSelect(8, buf))
            buf.WriteBool(connectParticles);
        if(WriteSelect(9, buf))
            buf.WriteBool(showPoints);
        if(WriteSelect(10, buf))
            buf.WriteString(indexVariable);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetStartIndex(buf.ReadInt());
            break;
        case 1:
            SetStopIndex(buf.ReadInt());
            break;
        case 2:
            SetStride(buf.ReadInt());
            break;
        case 3:
            SetStartPathType(buf.ReadInt());
            break;
        case 4:
            SetStopPathType(buf.ReadInt());
            break;
        case 5:
            SetTraceVariableX(buf.ReadString());
            break;
        case 6:
            SetTraceVariableY(buf.ReadString());
            break;
        case 7:
            SetTraceVariableZ(buf.ReadString());
            break;
        case 8:
            SetConnectParticles(buf.ReadBool());
            break;
        case 9:
            SetShowPoints(buf.ReadBool());
            break;
        case 10:
            SetIndexVariable(buf.ReadString());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + intToString("startIndex", startIndex, indent) + "\n";
        str = str + intToString("stopIndex", stopIndex, indent) + "\n";
        str = str + intToString("stride", stride, indent) + "\n";
        str = str + indent + "startPathType = ";
        if(startPathType == PATHTYPEENUM_ABSOLUTE)
            str = str + "PATHTYPEENUM_ABSOLUTE";
        if(startPathType == PATHTYPEENUM_RELATIVE)
            str = str + "PATHTYPEENUM_RELATIVE";
        str = str + "\n";
        str = str + indent + "stopPathType = ";
        if(stopPathType == PATHTYPEENUM_ABSOLUTE)
            str = str + "PATHTYPEENUM_ABSOLUTE";
        if(stopPathType == PATHTYPEENUM_RELATIVE)
            str = str + "PATHTYPEENUM_RELATIVE";
        str = str + "\n";
        str = str + stringToString("traceVariableX", traceVariableX, indent) + "\n";
        str = str + stringToString("traceVariableY", traceVariableY, indent) + "\n";
        str = str + stringToString("traceVariableZ", traceVariableZ, indent) + "\n";
        str = str + boolToString("connectParticles", connectParticles, indent) + "\n";
        str = str + boolToString("showPoints", showPoints, indent) + "\n";
        str = str + stringToString("indexVariable", indexVariable, indent) + "\n";
        return str;
    }


    // Attributes
    private int     startIndex;
    private int     stopIndex;
    private int     stride;
    private int     startPathType;
    private int     stopPathType;
    private String  traceVariableX;
    private String  traceVariableY;
    private String  traceVariableZ;
    private boolean connectParticles;
    private boolean showPoints;
    private String  indexVariable;
}

