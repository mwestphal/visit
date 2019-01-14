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

package llnl.visit.plots;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;
import llnl.visit.ColorAttribute;

// ****************************************************************************
// Class: HistogramAttributes
//
// Purpose:
//    Attributes for Histogram Plot
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class HistogramAttributes extends AttributeSubject implements Plugin
{
    private static int HistogramAttributes_numAdditionalAtts = 19;

    // Enum values
    public final static int OUTPUTTYPE_CURVE = 0;
    public final static int OUTPUTTYPE_BLOCK = 1;

    public final static int BASEDON_MANYVARSFORSINGLEZONE = 0;
    public final static int BASEDON_MANYZONESFORSINGLEVAR = 1;

    public final static int BINCONTRIBUTION_FREQUENCY = 0;
    public final static int BINCONTRIBUTION_WEIGHTED = 1;
    public final static int BINCONTRIBUTION_VARIABLE = 2;

    public final static int LIMITSMODE_ORIGINALDATA = 0;
    public final static int LIMITSMODE_CURRENTPLOT = 1;

    public final static int DATASCALE_LINEAR = 0;
    public final static int DATASCALE_LOG = 1;
    public final static int DATASCALE_SQUAREROOT = 2;


    public HistogramAttributes()
    {
        super(HistogramAttributes_numAdditionalAtts);

        basedOn = BASEDON_MANYZONESFORSINGLEVAR;
        histogramType = BINCONTRIBUTION_FREQUENCY;
        weightVariable = new String("default");
        limitsMode = LIMITSMODE_ORIGINALDATA;
        minFlag = false;
        maxFlag = false;
        min = 0;
        max = 1;
        numBins = 32;
        domain = 0;
        zone = 0;
        useBinWidths = true;
        outputType = OUTPUTTYPE_BLOCK;
        lineWidth = 0;
        color = new ColorAttribute(200, 80, 40);
        dataScale = DATASCALE_LINEAR;
        binScale = DATASCALE_LINEAR;
        normalizeHistogram = false;
        computeAsCDF = false;
    }

    public HistogramAttributes(int nMoreFields)
    {
        super(HistogramAttributes_numAdditionalAtts + nMoreFields);

        basedOn = BASEDON_MANYZONESFORSINGLEVAR;
        histogramType = BINCONTRIBUTION_FREQUENCY;
        weightVariable = new String("default");
        limitsMode = LIMITSMODE_ORIGINALDATA;
        minFlag = false;
        maxFlag = false;
        min = 0;
        max = 1;
        numBins = 32;
        domain = 0;
        zone = 0;
        useBinWidths = true;
        outputType = OUTPUTTYPE_BLOCK;
        lineWidth = 0;
        color = new ColorAttribute(200, 80, 40);
        dataScale = DATASCALE_LINEAR;
        binScale = DATASCALE_LINEAR;
        normalizeHistogram = false;
        computeAsCDF = false;
    }

    public HistogramAttributes(HistogramAttributes obj)
    {
        super(obj);

        basedOn = obj.basedOn;
        histogramType = obj.histogramType;
        weightVariable = new String(obj.weightVariable);
        limitsMode = obj.limitsMode;
        minFlag = obj.minFlag;
        maxFlag = obj.maxFlag;
        min = obj.min;
        max = obj.max;
        numBins = obj.numBins;
        domain = obj.domain;
        zone = obj.zone;
        useBinWidths = obj.useBinWidths;
        outputType = obj.outputType;
        lineWidth = obj.lineWidth;
        color = new ColorAttribute(obj.color);
        dataScale = obj.dataScale;
        binScale = obj.binScale;
        normalizeHistogram = obj.normalizeHistogram;
        computeAsCDF = obj.computeAsCDF;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return HistogramAttributes_numAdditionalAtts;
    }

    public boolean equals(HistogramAttributes obj)
    {
        // Create the return value
        return ((basedOn == obj.basedOn) &&
                (histogramType == obj.histogramType) &&
                (weightVariable.equals(obj.weightVariable)) &&
                (limitsMode == obj.limitsMode) &&
                (minFlag == obj.minFlag) &&
                (maxFlag == obj.maxFlag) &&
                (min == obj.min) &&
                (max == obj.max) &&
                (numBins == obj.numBins) &&
                (domain == obj.domain) &&
                (zone == obj.zone) &&
                (useBinWidths == obj.useBinWidths) &&
                (outputType == obj.outputType) &&
                (lineWidth == obj.lineWidth) &&
                (color == obj.color) &&
                (dataScale == obj.dataScale) &&
                (binScale == obj.binScale) &&
                (normalizeHistogram == obj.normalizeHistogram) &&
                (computeAsCDF == obj.computeAsCDF));
    }

    public String GetName() { return "Histogram"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetBasedOn(int basedOn_)
    {
        basedOn = basedOn_;
        Select(0);
    }

    public void SetHistogramType(int histogramType_)
    {
        histogramType = histogramType_;
        Select(1);
    }

    public void SetWeightVariable(String weightVariable_)
    {
        weightVariable = weightVariable_;
        Select(2);
    }

    public void SetLimitsMode(int limitsMode_)
    {
        limitsMode = limitsMode_;
        Select(3);
    }

    public void SetMinFlag(boolean minFlag_)
    {
        minFlag = minFlag_;
        Select(4);
    }

    public void SetMaxFlag(boolean maxFlag_)
    {
        maxFlag = maxFlag_;
        Select(5);
    }

    public void SetMin(double min_)
    {
        min = min_;
        Select(6);
    }

    public void SetMax(double max_)
    {
        max = max_;
        Select(7);
    }

    public void SetNumBins(int numBins_)
    {
        numBins = numBins_;
        Select(8);
    }

    public void SetDomain(int domain_)
    {
        domain = domain_;
        Select(9);
    }

    public void SetZone(int zone_)
    {
        zone = zone_;
        Select(10);
    }

    public void SetUseBinWidths(boolean useBinWidths_)
    {
        useBinWidths = useBinWidths_;
        Select(11);
    }

    public void SetOutputType(int outputType_)
    {
        outputType = outputType_;
        Select(12);
    }

    public void SetLineWidth(int lineWidth_)
    {
        lineWidth = lineWidth_;
        Select(13);
    }

    public void SetColor(ColorAttribute color_)
    {
        color = color_;
        Select(14);
    }

    public void SetDataScale(int dataScale_)
    {
        dataScale = dataScale_;
        Select(15);
    }

    public void SetBinScale(int binScale_)
    {
        binScale = binScale_;
        Select(16);
    }

    public void SetNormalizeHistogram(boolean normalizeHistogram_)
    {
        normalizeHistogram = normalizeHistogram_;
        Select(17);
    }

    public void SetComputeAsCDF(boolean computeAsCDF_)
    {
        computeAsCDF = computeAsCDF_;
        Select(18);
    }

    // Property getting methods
    public int            GetBasedOn() { return basedOn; }
    public int            GetHistogramType() { return histogramType; }
    public String         GetWeightVariable() { return weightVariable; }
    public int            GetLimitsMode() { return limitsMode; }
    public boolean        GetMinFlag() { return minFlag; }
    public boolean        GetMaxFlag() { return maxFlag; }
    public double         GetMin() { return min; }
    public double         GetMax() { return max; }
    public int            GetNumBins() { return numBins; }
    public int            GetDomain() { return domain; }
    public int            GetZone() { return zone; }
    public boolean        GetUseBinWidths() { return useBinWidths; }
    public int            GetOutputType() { return outputType; }
    public int            GetLineWidth() { return lineWidth; }
    public ColorAttribute GetColor() { return color; }
    public int            GetDataScale() { return dataScale; }
    public int            GetBinScale() { return binScale; }
    public boolean        GetNormalizeHistogram() { return normalizeHistogram; }
    public boolean        GetComputeAsCDF() { return computeAsCDF; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(basedOn);
        if(WriteSelect(1, buf))
            buf.WriteInt(histogramType);
        if(WriteSelect(2, buf))
            buf.WriteString(weightVariable);
        if(WriteSelect(3, buf))
            buf.WriteInt(limitsMode);
        if(WriteSelect(4, buf))
            buf.WriteBool(minFlag);
        if(WriteSelect(5, buf))
            buf.WriteBool(maxFlag);
        if(WriteSelect(6, buf))
            buf.WriteDouble(min);
        if(WriteSelect(7, buf))
            buf.WriteDouble(max);
        if(WriteSelect(8, buf))
            buf.WriteInt(numBins);
        if(WriteSelect(9, buf))
            buf.WriteInt(domain);
        if(WriteSelect(10, buf))
            buf.WriteInt(zone);
        if(WriteSelect(11, buf))
            buf.WriteBool(useBinWidths);
        if(WriteSelect(12, buf))
            buf.WriteInt(outputType);
        if(WriteSelect(13, buf))
            buf.WriteInt(lineWidth);
        if(WriteSelect(14, buf))
            color.Write(buf);
        if(WriteSelect(15, buf))
            buf.WriteInt(dataScale);
        if(WriteSelect(16, buf))
            buf.WriteInt(binScale);
        if(WriteSelect(17, buf))
            buf.WriteBool(normalizeHistogram);
        if(WriteSelect(18, buf))
            buf.WriteBool(computeAsCDF);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetBasedOn(buf.ReadInt());
            break;
        case 1:
            SetHistogramType(buf.ReadInt());
            break;
        case 2:
            SetWeightVariable(buf.ReadString());
            break;
        case 3:
            SetLimitsMode(buf.ReadInt());
            break;
        case 4:
            SetMinFlag(buf.ReadBool());
            break;
        case 5:
            SetMaxFlag(buf.ReadBool());
            break;
        case 6:
            SetMin(buf.ReadDouble());
            break;
        case 7:
            SetMax(buf.ReadDouble());
            break;
        case 8:
            SetNumBins(buf.ReadInt());
            break;
        case 9:
            SetDomain(buf.ReadInt());
            break;
        case 10:
            SetZone(buf.ReadInt());
            break;
        case 11:
            SetUseBinWidths(buf.ReadBool());
            break;
        case 12:
            SetOutputType(buf.ReadInt());
            break;
        case 13:
            SetLineWidth(buf.ReadInt());
            break;
        case 14:
            color.Read(buf);
            Select(14);
            break;
        case 15:
            SetDataScale(buf.ReadInt());
            break;
        case 16:
            SetBinScale(buf.ReadInt());
            break;
        case 17:
            SetNormalizeHistogram(buf.ReadBool());
            break;
        case 18:
            SetComputeAsCDF(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "basedOn = ";
        if(basedOn == BASEDON_MANYVARSFORSINGLEZONE)
            str = str + "BASEDON_MANYVARSFORSINGLEZONE";
        if(basedOn == BASEDON_MANYZONESFORSINGLEVAR)
            str = str + "BASEDON_MANYZONESFORSINGLEVAR";
        str = str + "\n";
        str = str + indent + "histogramType = ";
        if(histogramType == BINCONTRIBUTION_FREQUENCY)
            str = str + "BINCONTRIBUTION_FREQUENCY";
        if(histogramType == BINCONTRIBUTION_WEIGHTED)
            str = str + "BINCONTRIBUTION_WEIGHTED";
        if(histogramType == BINCONTRIBUTION_VARIABLE)
            str = str + "BINCONTRIBUTION_VARIABLE";
        str = str + "\n";
        str = str + stringToString("weightVariable", weightVariable, indent) + "\n";
        str = str + indent + "limitsMode = ";
        if(limitsMode == LIMITSMODE_ORIGINALDATA)
            str = str + "LIMITSMODE_ORIGINALDATA";
        if(limitsMode == LIMITSMODE_CURRENTPLOT)
            str = str + "LIMITSMODE_CURRENTPLOT";
        str = str + "\n";
        str = str + boolToString("minFlag", minFlag, indent) + "\n";
        str = str + boolToString("maxFlag", maxFlag, indent) + "\n";
        str = str + doubleToString("min", min, indent) + "\n";
        str = str + doubleToString("max", max, indent) + "\n";
        str = str + intToString("numBins", numBins, indent) + "\n";
        str = str + intToString("domain", domain, indent) + "\n";
        str = str + intToString("zone", zone, indent) + "\n";
        str = str + boolToString("useBinWidths", useBinWidths, indent) + "\n";
        str = str + indent + "outputType = ";
        if(outputType == OUTPUTTYPE_CURVE)
            str = str + "OUTPUTTYPE_CURVE";
        if(outputType == OUTPUTTYPE_BLOCK)
            str = str + "OUTPUTTYPE_BLOCK";
        str = str + "\n";
        str = str + intToString("lineWidth", lineWidth, indent) + "\n";
        str = str + indent + "color = {" + color.Red() + ", " + color.Green() + ", " + color.Blue() + ", " + color.Alpha() + "}\n";
        str = str + indent + "dataScale = ";
        if(dataScale == DATASCALE_LINEAR)
            str = str + "DATASCALE_LINEAR";
        if(dataScale == DATASCALE_LOG)
            str = str + "DATASCALE_LOG";
        if(dataScale == DATASCALE_SQUAREROOT)
            str = str + "DATASCALE_SQUAREROOT";
        str = str + "\n";
        str = str + indent + "binScale = ";
        if(binScale == DATASCALE_LINEAR)
            str = str + "DATASCALE_LINEAR";
        if(binScale == DATASCALE_LOG)
            str = str + "DATASCALE_LOG";
        if(binScale == DATASCALE_SQUAREROOT)
            str = str + "DATASCALE_SQUAREROOT";
        str = str + "\n";
        str = str + boolToString("normalizeHistogram", normalizeHistogram, indent) + "\n";
        str = str + boolToString("computeAsCDF", computeAsCDF, indent) + "\n";
        return str;
    }


    // Attributes
    private int            basedOn;
    private int            histogramType;
    private String         weightVariable;
    private int            limitsMode;
    private boolean        minFlag;
    private boolean        maxFlag;
    private double         min;
    private double         max;
    private int            numBins;
    private int            domain;
    private int            zone;
    private boolean        useBinWidths;
    private int            outputType;
    private int            lineWidth;
    private ColorAttribute color;
    private int            dataScale;
    private int            binScale;
    private boolean        normalizeHistogram;
    private boolean        computeAsCDF;
}

