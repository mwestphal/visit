/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef LABELATTRIBUTES_H
#define LABELATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <FontAttributes.h>

// ****************************************************************************
// Class: LabelAttributes
//
// Purpose:
//    This class contains the fields that we need to set the attributes for the Label plot.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class LabelAttributes : public AttributeSubject
{
public:
    enum LabelIndexDisplay
    {
        Natural,
        LogicalIndex,
        Index
    };
    enum LabelHorizontalAlignment
    {
        HCenter,
        Left,
        Right
    };
    enum LabelVerticalAlignment
    {
        VCenter,
        Top,
        Bottom
    };
    enum LabelDrawFacing
    {
        Front,
        Back,
        FrontAndBack
    };
    enum VariableType
    {
        LABEL_VT_MESH,
        LABEL_VT_SCALAR_VAR,
        LABEL_VT_VECTOR_VAR,
        LABEL_VT_TENSOR_VAR,
        LABEL_VT_SYMMETRIC_TENSOR_VAR,
        LABEL_VT_ARRAY_VAR,
        LABEL_VT_LABEL_VAR,
        LABEL_VT_MATERIAL,
        LABEL_VT_SUBSET,
        LABEL_VT_UNKNOWN_TYPE
    };
    enum DepthTestMode
    {
        LABEL_DT_AUTO,
        LABEL_DT_ALWAYS,
        LABEL_DT_NEVER
    };

    // These constructors are for objects of this class
    LabelAttributes();
    LabelAttributes(const LabelAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    LabelAttributes(private_tmfs_t tmfs);
    LabelAttributes(const LabelAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~LabelAttributes();

    virtual LabelAttributes& operator = (const LabelAttributes &obj);
    virtual bool operator == (const LabelAttributes &obj) const;
    virtual bool operator != (const LabelAttributes &obj) const;
private:
    void Init();
    void Copy(const LabelAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectTextFont1();
    void SelectTextFont2();
    void SelectFormatTemplate();

    // Property setting methods
    void SetVarType(VariableType varType_);
    void SetLegendFlag(bool legendFlag_);
    void SetShowNodes(bool showNodes_);
    void SetShowCells(bool showCells_);
    void SetRestrictNumberOfLabels(bool restrictNumberOfLabels_);
    void SetDrawLabelsFacing(LabelDrawFacing drawLabelsFacing_);
    void SetLabelDisplayFormat(LabelIndexDisplay labelDisplayFormat_);
    void SetNumberOfLabels(int numberOfLabels_);
    void SetTextFont1(const FontAttributes &textFont1_);
    void SetTextFont2(const FontAttributes &textFont2_);
    void SetHorizontalJustification(LabelHorizontalAlignment horizontalJustification_);
    void SetVerticalJustification(LabelVerticalAlignment verticalJustification_);
    void SetDepthTestMode(DepthTestMode depthTestMode_);
    void SetFormatTemplate(const std::string &formatTemplate_);

    // Property getting methods
    VariableType         GetVarType() const;
    bool                 GetLegendFlag() const;
    bool                 GetShowNodes() const;
    bool                 GetShowCells() const;
    bool                 GetRestrictNumberOfLabels() const;
    LabelDrawFacing      GetDrawLabelsFacing() const;
    LabelIndexDisplay    GetLabelDisplayFormat() const;
    int                  GetNumberOfLabels() const;
    const FontAttributes &GetTextFont1() const;
          FontAttributes &GetTextFont1();
    const FontAttributes &GetTextFont2() const;
          FontAttributes &GetTextFont2();
    LabelHorizontalAlignment GetHorizontalJustification() const;
    LabelVerticalAlignment GetVerticalJustification() const;
    DepthTestMode        GetDepthTestMode() const;
    const std::string    &GetFormatTemplate() const;
          std::string    &GetFormatTemplate();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string LabelIndexDisplay_ToString(LabelIndexDisplay);
    static bool LabelIndexDisplay_FromString(const std::string &, LabelIndexDisplay &);
protected:
    static std::string LabelIndexDisplay_ToString(int);
public:
    static std::string LabelHorizontalAlignment_ToString(LabelHorizontalAlignment);
    static bool LabelHorizontalAlignment_FromString(const std::string &, LabelHorizontalAlignment &);
protected:
    static std::string LabelHorizontalAlignment_ToString(int);
public:
    static std::string LabelVerticalAlignment_ToString(LabelVerticalAlignment);
    static bool LabelVerticalAlignment_FromString(const std::string &, LabelVerticalAlignment &);
protected:
    static std::string LabelVerticalAlignment_ToString(int);
public:
    static std::string LabelDrawFacing_ToString(LabelDrawFacing);
    static bool LabelDrawFacing_FromString(const std::string &, LabelDrawFacing &);
protected:
    static std::string LabelDrawFacing_ToString(int);
public:
    static std::string VariableType_ToString(VariableType);
    static bool VariableType_FromString(const std::string &, VariableType &);
protected:
    static std::string VariableType_ToString(int);
public:
    static std::string DepthTestMode_ToString(DepthTestMode);
    static bool DepthTestMode_FromString(const std::string &, DepthTestMode &);
protected:
    static std::string DepthTestMode_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    virtual bool ChangesRequireRecalculation(const LabelAttributes &) const;
    virtual bool VarChangeRequiresReset(void);
    virtual void ProcessOldVersions(DataNode *parentNode, const char *configVersion);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_varType = 0,
        ID_legendFlag,
        ID_showNodes,
        ID_showCells,
        ID_restrictNumberOfLabels,
        ID_drawLabelsFacing,
        ID_labelDisplayFormat,
        ID_numberOfLabels,
        ID_textFont1,
        ID_textFont2,
        ID_horizontalJustification,
        ID_verticalJustification,
        ID_depthTestMode,
        ID_formatTemplate,
        ID__LAST
    };

private:
    int            varType;
    bool           legendFlag;
    bool           showNodes;
    bool           showCells;
    bool           restrictNumberOfLabels;
    int            drawLabelsFacing;
    int            labelDisplayFormat;
    int            numberOfLabels;
    FontAttributes textFont1;
    FontAttributes textFont2;
    int            horizontalJustification;
    int            verticalJustification;
    int            depthTestMode;
    std::string    formatTemplate;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define LABELATTRIBUTES_TMFS "ibbbbiiiaaiiis"

#endif
