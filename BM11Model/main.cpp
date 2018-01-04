#include <stdio.h>
#include <GLKit/GLKMath.h>

/*
                        O
                       /|\
                      / | \
                     /  |  \
                    /   |   \
                   /    |    \
                  /     |     \
                 /      |      \
                /       |       \
               /        |        \
              /         |         \
             /         -Bn         \
            /       -/     \-       \
           /     -/           \-     \
          /   -/                 \-   \
         / -/                       \- \
        /. . . . . . . . . . . . . . . .\
      An                                  Cn

      Geometric Constraints:
      ---------------------
      - An, Bn, Cn and O are vertices of an irregular tetrahedron
      - The overall structure has two identical tetrahedrons with O as common vertex
      - OBA, OBC are identical scalene triangles
      - ABC is an isoceles triangle
      - AOC is an isoceles triangle
      - length(OA) = length(OC)
      - length(BA) = length(BC)

 */


class BM11Model {
public:
    BM11Model(void) {
        setInputParameters(BM11Model::getDefaultInputParameters());
    };

    ~BM11Model(void) {
    };

    typedef struct {
        float      squareSideLength;   /* Size of the starting square forming one of the triangles */
        float      baseCutBackLength;  /* Cut back on base of square to form the triangle          */
        float      angle_ABC;          /* Angle on the ground plane between the two triangles      */
        GLKVector2 frameCrossSection;  /* Cross section dimensions (in)                            */
        float      frameWallThickness; /* Wall thickness (in)                                      */
        float      metalDensity;       /* lb/in^3                                                  */
        float      shoulderHeight;     /* ft                                                       */
        float      mirrorBoltSpacing;  /* ft                                                       */

        struct {
            float frameMetal;            /* $ / ft   */
            float mirror;                /* $ / ft^2 */
            float mirrorBolt;            /* $ / each */
            float frameThroughHoleDrill; /* $ / each */
            float frameThroughHoleTap;   /* $ / each */
        } unit_cost;

    } InputParameters;
    
    typedef struct {
        struct {
            float OB;
            float BA;
            float OA;
            float AC;
        } edge_length;
        struct {
            float angle_OAB;
            float angle_AOB;
            float angle_ABO;
            float angle_OCB;
            float angle_COB;
            float angle_CBO;
            float angle_ABC;
            float angle_BAC;
            float angle_BCA;
            float angle_AOC;
            float angle_OAC;
            float angle_OCA;
        } vertex_angle;
        struct {
            GLKVector3 A0;
            GLKVector3 C0;
            GLKVector3 B0;
            GLKVector3 O;
            GLKVector3 A1;
            GLKVector3 B1;
            GLKVector3 C1;
        } vertex_coord;
        struct {
            GLKVector2 footprint;
            float      footprint_area;
            float      footprint_aspect_ratio;
            float      height;
            float      triangle_area;
            float      walkway_top_angle;
            float      walkway_base_width;
            float      walkway_shoulder_width;
        } overall_structure;
        struct {
            float      angle_BOA_BOC;
            float      angle_BOA_ABC;
        } dihedral_angle;
        struct { 
            float      perimeter_length;
            float      reinforce_length;
            float      total_length;
            float      metal_volume;
            float      metal_mass;
            float      metal_cost;
            float      drill_count; /* Through drill,  both sides of box section */
            float      drill_cost;
            float      tap_count;
            float      tap_cost;
        } frame;
        struct {
            float      surface_area;
            float      cost;
            float      bolt_count;
            float      bolt_cost;
        } mirror;
        struct {
            float      total_surface_area_XY;
            float      total_surface_area_YZ;
        } wind;
        struct {
            float mass;
            float cost;
        } total;
    } OutputParameters;

    static InputParameters getDefaultInputParameters(void);
    static void            printInputParameters(InputParameters params);
    static void            printOutputParameters(OutputParameters params);

    void                   setInputParameters(InputParameters params) { _inputParams = params; _is_dirty = true; };
    InputParameters        getInputParameters(void)                   { return _inputParams; };
    
    OutputParameters       getOutputParameters(void) { 
        if (_is_dirty) {
            evaluate();
            _is_dirty = false;
        }
        return _outputParams;
    }
    
private:
    bool evaluate(void);
    
    InputParameters  _inputParams;
    OutputParameters _outputParams;
    bool             _is_dirty;
};

float sq(float x)
{
    return (x * x);
}

void GLKVector3Print(GLKVector3 v)
{
    printf("[ %.3f, %.3f, %.3f ]\n", v.x, v.y, v.z);
}

float MPH_To_FTSec(float mph)
{
    return mph * 1.46667f;
}

BM11Model::InputParameters BM11Model::getDefaultInputParameters(void)
{
    InputParameters p;

    p.squareSideLength     = 16.0f;
    p.baseCutBackLength    = 2.0f;
    p.angle_ABC            = GLKMathDegreesToRadians(110.0f);
    p.frameCrossSection    = GLKVector2Make(0.75f, 1.5f); /* in      */
    p.frameWallThickness   = (1.0f / 16.0f);              /* in      */
    p.metalDensity         = 0.289f;                      /* lb/in^3 */
    p.shoulderHeight       = 5.0f;                        /* ft      */
    p.mirrorBoltSpacing    = 2.0f;                        /* ft      */

    p.unit_cost.frameMetal = 4.4f;                    /* $ / ft   */
    p.unit_cost.mirror     = (220.0f / (8.0f * 4.0)); /* $ / ft^2 */
    /* McMaster Part #90585A537
       316 Stainless Steel Hex Drive Flat Head Screw, 
       82 Degree Countersink Angle, 1/4"-20 Thread Size, 1/2" Long.
       Sold in bags of 10 */
    p.unit_cost.mirrorBolt = (3.67f / 10.0f); /* $ / each */
    p.unit_cost.frameThroughHoleDrill = (695.0f / 160.0f); /* $695 for 160 holes at 1/4" dia from Bayshore */
    p.unit_cost.frameThroughHoleTap   = (480.0f / 320.0f); /* $480 for 320 tap plunges 1/4-20 from Bayshore */

    return p;
}

void BM11Model::printInputParameters(InputParameters params)
{
    
}

void BM11Model::printOutputParameters(OutputParameters params)
{
    OutputParameters *op = &params;

    printf("Edge lengths:\n");
    printf("   OB = %.3f ft\n", op->edge_length.OB);
    printf("   BA = %.3f ft\n", op->edge_length.BA);
    printf("   OA = %.3f ft\n", op->edge_length.OA);
    printf("   AC = %.3f ft\n", op->edge_length.AC);

    printf("Scalene triangle OBA and OBC vertex angles:\n");
    printf("   angle_OAB = angle_OCB = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_OAB));
    printf("   angle_AOB = angle_COB = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_AOB));
    printf("   angle_ABO = angle_CBO = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_ABO));

    printf("Isoceles triangle ABC vertex angles:\n");
    printf("   angle_ABC             = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_ABC));
    printf("   angle_BAC = angle_BCA = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_BAC));

    printf("Isoceles triangle AOC vertex angles:\n");
    printf("   angle_AOC             = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_AOC));
    printf("   angle_OAC = angle_OCA = %.3f degrees\n", GLKMathRadiansToDegrees(op->vertex_angle.angle_OAC));

    printf("Vertex coordinates:\n");
    printf("   O  = "); GLKVector3Print(op->vertex_coord.O);
    printf("   B0 = "); GLKVector3Print(op->vertex_coord.B0);
    printf("   A0 = "); GLKVector3Print(op->vertex_coord.A0);
    printf("   C0 = "); GLKVector3Print(op->vertex_coord.C0);
    printf("   B1 = "); GLKVector3Print(op->vertex_coord.B1);
    printf("   A1 = "); GLKVector3Print(op->vertex_coord.A1);
    printf("   C1 = "); GLKVector3Print(op->vertex_coord.C1);

    printf("Structural shape summary:\n");
    printf("   Footprint dimensions   = [%.3f, %.3f] ft\n", op->overall_structure.footprint.x, op->overall_structure.footprint.y);
    printf("   Footprint surface area = %.3f ft^2\n", op->overall_structure.footprint_area);
    printf("   Footprint aspect ratio = %.3f\n", op->overall_structure.footprint_aspect_ratio);
    printf("   Height                 = %.3f ft\n", op->overall_structure.height);
    printf("   Triangle surface area  = %.3f ft^2\n", op->overall_structure.triangle_area);
    printf("   Walkway top angle      = %.3f degrees\n", GLKMathRadiansToDegrees(op->overall_structure.walkway_top_angle));
    printf("   Walkway base width     = %.3f ft\n", op->overall_structure.walkway_base_width);
    printf("   Walkway shoulder width = %.3f ft\n", op->overall_structure.walkway_shoulder_width);

    printf("Important dihedral angles:\n");
    printf("   Between triangle pairs (angle_BOA_BOC)      = %.3f degrees\n", GLKMathRadiansToDegrees(op->dihedral_angle.angle_BOA_BOC));
    printf("   Between triangle and ground (angle_BOA_ABC) = %.3f degrees\n", GLKMathRadiansToDegrees(op->dihedral_angle.angle_BOA_ABC));

    printf("Frame info:\n");
    printf("   Perimeter length         = %.3f ft\n", op->frame.perimeter_length);
    printf("   Reinforce length         = %.3f ft\n", op->frame.reinforce_length);
    printf("   Total length             = %.3f ft\n", op->frame.total_length);
    printf("   Metal volume             = %.3f in^3 (%.3f ft^3)\n", op->frame.metal_volume, op->frame.metal_volume / (12.0f * 12.0f * 12.0f));
    printf("   Metal Mass               = %.3f lb\n", op->frame.metal_mass);
    printf("   Metal Cost               = $%.3f\n", op->frame.metal_cost);
    printf("   Drill Count              = %.3f\n", op->frame.drill_count);
    printf("   Drill Cost               = $%.3f\n", op->frame.drill_cost);
    printf("   Tap Count                = %.3f\n", op->frame.tap_count);
    printf("   Tap Cost                 = $%.3f\n", op->frame.tap_cost);
    
    printf("Mirror coating info:\n");
    printf("   Total surface area       = %.3f ft^2\n", op->mirror.surface_area);
    printf("   Mirror cost              = $%.3f\n", op->mirror.cost);
    printf("   Mirror bolt count        = %f\n", op->mirror.bolt_count);
    printf("   Mirror bolt cost         = $%.3f\n", op->mirror.bolt_cost);
    
    printf("Wind:\n");
    printf("   XY plane:\n");
    printf("      Total surface area = %.3f ft^2\n", op->wind.total_surface_area_XY);
    for (float mph = 5.0f; mph <= 100.0f; mph += 5.0f) {
        float pressure   = sq(MPH_To_FTSec(mph)) * 0.00256; /* lb/ft^2 */
        float coeff_drag = 1.0f;
        float force      = (op->wind.total_surface_area_XY * pressure * coeff_drag);
        printf("      Side force at %f MPH = %.0f lbs\n", mph, force);
    }
    printf("   YZ plane:\n");
    printf("      Total surface area = %.3f ft^2\n", op->wind.total_surface_area_YZ);
    for (float mph = 5.0f; mph <= 100.0f; mph += 5.0f) {
        float pressure   = sq(MPH_To_FTSec(mph)) * 0.00256; /* lb/ft^2 */
        float coeff_drag = 1.0f;
        float force      = (op->wind.total_surface_area_YZ * pressure * coeff_drag);
        printf("      Side force at %f MPH = %.0f lbs\n", mph, force);
    }

    printf("Total:\n");
    printf("   Mass: %.3f lb\n", op->total.mass);
    printf("   Cost: $%.3f\n",   op->total.cost);
}

bool BM11Model::evaluate(void)
{
    OutputParameters *op = &_outputParams;

    memset(op, 0, sizeof(*op));

    /* Calculate lengths of all tetrahedron edges */
    op->edge_length.OB = sqrtf((_inputParams.squareSideLength * _inputParams.squareSideLength) +
                              (_inputParams.baseCutBackLength * _inputParams.baseCutBackLength));
    op->edge_length.BA = (_inputParams.squareSideLength - _inputParams.baseCutBackLength);
    op->edge_length.OA = sqrtf(2.0f * _inputParams.squareSideLength * _inputParams.squareSideLength);
    op->edge_length.AC = (2.0f * op->edge_length.BA * sin(_inputParams.angle_ABC * 0.5f));

    /* Compute angles of all triangles in the tetrahedron */
    /* Scalene triangles OBA and OBC: computed via triangle cosine law */
    op->vertex_angle.angle_OAB = acosf((sq(op->edge_length.OA) + sq(op->edge_length.BA) - sq(op->edge_length.OB)) /
                                      (2.0f * op->edge_length.OA * op->edge_length.BA));
    op->vertex_angle.angle_AOB = acosf((sq(op->edge_length.OA) + sq(op->edge_length.OB) - sq(op->edge_length.BA)) /
                                      (2.0f * op->edge_length.OA * op->edge_length.OB));
    op->vertex_angle.angle_ABO = (M_PI - op->vertex_angle.angle_OAB - op->vertex_angle.angle_AOB);
    op->vertex_angle.angle_OCB = op->vertex_angle.angle_OAB;
    op->vertex_angle.angle_COB = op->vertex_angle.angle_AOB;
    op->vertex_angle.angle_CBO = op->vertex_angle.angle_ABO;

    /* Isoceles triangle ABC */
    op->vertex_angle.angle_ABC = _inputParams.angle_ABC;
    op->vertex_angle.angle_BAC = (M_PI - op->vertex_angle.angle_ABC) * 0.5f;
    op->vertex_angle.angle_BCA = op->vertex_angle.angle_BAC;

    /* Isoceles triangle AOC */
    op->vertex_angle.angle_AOC = (2.0f * asin(op->edge_length.AC / (2.0f * op->edge_length.OA)));
    op->vertex_angle.angle_OAC = (M_PI - op->vertex_angle.angle_AOC) * 0.5f;
    op->vertex_angle.angle_OCA = op->vertex_angle.angle_OAC;

    /* Validate the tetrahedron via the 3D law of sines */
    {
        float epsilon = (1.0f / 1000.0f);
        /* O,ABC */
        {
            float lhs = (sin(op->vertex_angle.angle_OAC) * sin(op->vertex_angle.angle_OCB) * sin(op->vertex_angle.angle_ABO));
            float rhs = (sin(op->vertex_angle.angle_OCA) * sin(op->vertex_angle.angle_CBO) * sin(op->vertex_angle.angle_OAB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* A,OBC */
        {
            float lhs = (sin(op->vertex_angle.angle_AOC) * sin(op->vertex_angle.angle_BCA) * sin(op->vertex_angle.angle_ABO));
            float rhs = (sin(op->vertex_angle.angle_OCA) * sin(op->vertex_angle.angle_ABC) * sin(op->vertex_angle.angle_AOB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* B,AOC */
        {
            float lhs = (sin(op->vertex_angle.angle_BAC) * sin(op->vertex_angle.angle_OCB) * sin(op->vertex_angle.angle_AOB));
            float rhs = (sin(op->vertex_angle.angle_BCA) * sin(op->vertex_angle.angle_COB) * sin(op->vertex_angle.angle_OAB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* C,ABO */
        {
            float lhs = (sin(op->vertex_angle.angle_OAC) * sin(op->vertex_angle.angle_COB) * sin(op->vertex_angle.angle_ABC));
            float rhs = (sin(op->vertex_angle.angle_AOC) * sin(op->vertex_angle.angle_CBO) * sin(op->vertex_angle.angle_BAC));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
    }
        
    /* Compute vertex positions for the first tetrahedron */
    float      length_AM = op->edge_length.AC * 0.5f; /* M is at the midpoint between A and C */
    op->vertex_coord.A0 = GLKVector3Make(-length_AM, 0, 0);
    op->vertex_coord.C0 = GLKVector3Make(+length_AM, 0, 0);
    op->vertex_coord.B0 = GLKVector3Make(0, 0, sqrtf(sq(op->edge_length.BA) - sq(length_AM)));
    op->vertex_coord.O.x = 0;
    op->vertex_coord.O.z = ((sq(op->edge_length.OB) - sq(op->edge_length.OA) + sq(length_AM) - sq(op->vertex_coord.B0.z)) / 
           (-2.0f * op->vertex_coord.B0.z));
    op->vertex_coord.O.y = sqrtf(sq(op->edge_length.OA) - sq(length_AM) - sq(op->vertex_coord.O.z));

    /* Translate first tetrahedron such that O.xz = 0 */
    op->vertex_coord.A0.z -= op->vertex_coord.O.z;
    op->vertex_coord.B0.z -= op->vertex_coord.O.z;
    op->vertex_coord.C0.z -= op->vertex_coord.O.z;
    op->vertex_coord.O.z = 0;
    
    /* Second tetrahedron ABC is just first tetrahedron ABC with Z negated */
    op->vertex_coord.A1 = GLKVector3Multiply(op->vertex_coord.A0, GLKVector3Make(1.0f, 1.0f, -1.0f));
    op->vertex_coord.B1 = GLKVector3Multiply(op->vertex_coord.B0, GLKVector3Make(1.0f, 1.0f, -1.0f));
    op->vertex_coord.C1 = GLKVector3Multiply(op->vertex_coord.C0, GLKVector3Make(1.0f, 1.0f, -1.0f));

    /* Overall structure stats */
    op->overall_structure.footprint              = GLKVector2Make(op->vertex_coord.C0.x - op->vertex_coord.A0.x,
                                                                 op->vertex_coord.A1.z - op->vertex_coord.A0.z);
    op->overall_structure.footprint_area         = (op->overall_structure.footprint.x * op->overall_structure.footprint.y);
    op->overall_structure.footprint_aspect_ratio = (op->overall_structure.footprint.x / op->overall_structure.footprint.y);
    op->overall_structure.height                 = op->vertex_coord.O.y;
    op->overall_structure.triangle_area          = GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(op->vertex_coord.O, op->vertex_coord.B0),
                                                                                          GLKVector3Subtract(op->vertex_coord.A0, op->vertex_coord.B0))) * 0.5f;
    op->overall_structure.walkway_top_angle      = atanf(op->vertex_coord.B1.z / op->vertex_coord.O.y) * 2.0f;
    op->overall_structure.walkway_base_width     = op->vertex_coord.B1.z - op->vertex_coord.B0.z;
    op->overall_structure.walkway_shoulder_width = ((op->vertex_coord.O.y - _inputParams.shoulderHeight) * op->vertex_coord.B1.z * 2.0f) / op->vertex_coord.O.y;

    /* Important dihedral angles */
    GLKVector3 BO            = GLKVector3Subtract(op->vertex_coord.B0, op->vertex_coord.O);
    GLKVector3 BA            = GLKVector3Subtract(op->vertex_coord.B0, op->vertex_coord.A0);
    GLKVector3 BC            = GLKVector3Subtract(op->vertex_coord.B0, op->vertex_coord.C0);
    GLKVector3 norm_BOA      = GLKVector3Normalize(GLKVector3CrossProduct(BO, BA));
    GLKVector3 norm_BOC      = GLKVector3Normalize(GLKVector3CrossProduct(BO, BC));
    GLKVector3 norm_ABC      = GLKVector3Make(0.0f, 1.0f, 0.0f);
    op->dihedral_angle.angle_BOA_BOC = acosf(GLKVector3DotProduct(norm_BOA, norm_BOC));
    op->dihedral_angle.angle_BOA_ABC = acosf(GLKVector3DotProduct(norm_BOA, norm_ABC));

    /* Frame info */
    op->frame.perimeter_length    = ((op->edge_length.BA * 4) + 
                                    (op->edge_length.OA * 4) + 
                                    (op->edge_length.OB * 4));
    /* Shitty estimate on reinforcement.  Need to design first, but assuming 3 per triangle, so this is approx.
       Also, one cross-bar between B0 and B1 */
    op->frame.reinforce_length    = (op->edge_length.BA * 1.6 * 4) + op->overall_structure.walkway_base_width;
    op->frame.total_length        = (op->frame.perimeter_length + op->frame.reinforce_length);
    float      xsection_area      = (_inputParams.frameCrossSection.x *
                                     _inputParams.frameCrossSection.y);
    GLKVector2 xsection_inner_dim = GLKVector2Make(_inputParams.frameCrossSection.x - (_inputParams.frameWallThickness * 2.0f),
                                                   _inputParams.frameCrossSection.y - (_inputParams.frameWallThickness * 2.0f));
    float      xsection_inner_area = (xsection_inner_dim.x * xsection_inner_dim.y);
    float      xsection_metal_area = (xsection_area - xsection_inner_area);
    op->frame.metal_volume        = (xsection_metal_area * (op->frame.total_length * 12.0f));
    op->frame.metal_mass          = (op->frame.metal_volume * _inputParams.metalDensity);
    op->frame.metal_cost          = (op->frame.total_length * _inputParams.unit_cost.frameMetal);
    op->frame.drill_count         = (op->frame.total_length / _inputParams.mirrorBoltSpacing);
    op->frame.drill_cost          = (op->frame.drill_count * _inputParams.unit_cost.frameThroughHoleDrill);
    op->frame.tap_count           = (op->frame.drill_count * 2.0f); /* x 2 due to double sided mirror attachment */
    op->frame.tap_cost            = (op->frame.tap_count * _inputParams.unit_cost.frameThroughHoleTap);

    /* Mirror assembly */
    op->mirror.surface_area = (op->overall_structure.triangle_area * 8.0f); /* 8 triangles */
    op->mirror.cost         = (op->mirror.surface_area * _inputParams.unit_cost.mirror);
    op->mirror.bolt_count   = op->frame.tap_count;
    op->mirror.bolt_cost    = (op->mirror.bolt_count * _inputParams.unit_cost.mirrorBolt);
    
    /* Wind force calculations */
    /* Project triangle(OBA) onto XY plane */
    GLKVector3 ApXY = GLKVector3Multiply(op->vertex_coord.A0, GLKVector3Make(1.0f, 1.0f, 0.0f));
    GLKVector3 BpXY = GLKVector3Multiply(op->vertex_coord.B0, GLKVector3Make(1.0f, 1.0f, 0.0f));
    GLKVector3 OpXY = GLKVector3Multiply(op->vertex_coord.O,  GLKVector3Make(1.0f, 1.0f, 0.0f));
    /* Compute projected surface area */
    float      triangle_surface_area_XY = (GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(BpXY, ApXY),
                                                                                   GLKVector3Subtract(BpXY, OpXY))) * 0.5f);
    op->wind.total_surface_area_XY = (triangle_surface_area_XY * 2.0f);
    /* Project triangle(OBA) onto YZ plane */
    GLKVector3 ApYZ = GLKVector3Multiply(op->vertex_coord.A0, GLKVector3Make(0.0f, 1.0f, 1.0f));
    GLKVector3 BpYZ = GLKVector3Multiply(op->vertex_coord.B0, GLKVector3Make(0.0f, 1.0f, 1.0f));
    GLKVector3 OpYZ = GLKVector3Multiply(op->vertex_coord.O,  GLKVector3Make(0.0f, 1.0f, 1.0f));
    /* Compute projected surface area */
    float      triangle_surface_area_YZ = (GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(BpYZ, ApYZ),
                                                                                   GLKVector3Subtract(BpYZ, OpYZ))) * 0.5f);
    op->wind.total_surface_area_YZ = (triangle_surface_area_YZ * 2.0f);

    
    /* Totals */
    op->total.mass = op->frame.metal_mass; /* TODO: Mirror and bolts */
    op->total.cost = (op->frame.metal_cost + 
                      op->frame.drill_cost + 
                      op->frame.tap_cost   + 
                      op->mirror.cost      + 
                      op->mirror.bolt_cost);

    return false;
}

int main(void)
{
    BM11Model model;

    /* Evaluate model with default inputs and dump out everything */
    if ((1)) {
        BM11Model::OutputParameters output_params = model.getOutputParameters();
        BM11Model::printOutputParameters(output_params);
    }

    /* Sweep a particular input parameter and dump single output */
    if ((0)) {
        BM11Model::InputParameters input_params = BM11Model::getDefaultInputParameters();
        printf("squareSideLength, TotalCost\n");
        for (input_params.squareSideLength = 16.0f; input_params.squareSideLength >= 8.0f; input_params.squareSideLength -= 0.5f) {
            model.setInputParameters(input_params);
            BM11Model::OutputParameters output_params = model.getOutputParameters();
            printf("%.2f, %.2f\n", input_params.squareSideLength, output_params.total.cost);
        }
    }

    return 0;
}


