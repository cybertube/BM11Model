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
        float      squareSideLength;       /* Size of the starting square forming one of the triangles */
        float      baseCutBackLength;      /* Cut back on base of square to form the triangle          */
        float      angle_ABC;              /* Angle on the ground plane between the two triangles      */
        GLKVector2 frameCrossSection;      /* Cross section dimensions (in)                            */
        float      frameWallThickness;     /* Wall thickness (in)                                      */
        float      metalDensity;           /* lb/in^3                                                  */
        float      metalCost;              /* $ / ft                                                   */
    } InputParameters;
    
    typedef struct {
        
    } OutputParameters;

    static InputParameters getDefaultInputParameters(void);

    void            setInputParameters(InputParameters p) { _inputParams = p; };
    InputParameters getInputParameters(void)              { return _inputParams; };

    bool            evaluate(void);


private:
    InputParameters _inputParams;
};

BM11Model::InputParameters BM11Model::getDefaultInputParameters(void)
{
    InputParameters p;

    p.squareSideLength  = 16.0f;
    p.baseCutBackLength = 2.0f;
    p.angle_ABC         = GLKMathDegreesToRadians(110.0f);

    return p;
}

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

bool BM11Model::evaluate(void)
{
    /* Calculate lengths of all tetrahedron edges */
    float length_OB = sqrtf((_inputParams.squareSideLength * _inputParams.squareSideLength) +
                            (_inputParams.baseCutBackLength * _inputParams.baseCutBackLength));
    float length_BA = (_inputParams.squareSideLength - _inputParams.baseCutBackLength);
    float length_OA = sqrtf(2.0f * _inputParams.squareSideLength * _inputParams.squareSideLength);
    float length_AC = (2.0f * length_BA * sin(_inputParams.angle_ABC * 0.5f));

    printf("Edge lengths:\n");
    printf("   length_OB = %.3f ft\n", length_OB);
    printf("   length_BA = %.3f ft\n", length_BA);
    printf("   length_OA = %.3f ft\n", length_OA);
    printf("   length_AC = %.3f ft\n", length_AC);

    /* Compute angles of all triangles in the tetrahedron */
    /* Scalene triangles OBA and OBC: computed via triangle cosine law */
    float angle_OAB = acosf((sq(length_OA) + sq(length_BA) - sq(length_OB)) /
                            (2.0f * length_OA * length_BA));
    float angle_AOB = acosf((sq(length_OA) + sq(length_OB) - sq(length_BA)) /
                            (2.0f * length_OA * length_OB));
    float angle_ABO = (M_PI - angle_OAB - angle_AOB);
    float angle_OCB = angle_OAB;
    float angle_COB = angle_AOB;
    float angle_CBO = angle_ABO;
    printf("Scalene triangle OBA and OBC vertex angles:\n");
    printf("   angle_OAB = angle_OCB = %.3f degrees\n", GLKMathRadiansToDegrees(angle_OAB));
    printf("   angle_AOB = angle_COB = %.3f degrees\n", GLKMathRadiansToDegrees(angle_AOB));
    printf("   angle_ABO = angle_CBO = %.3f degrees\n", GLKMathRadiansToDegrees(angle_ABO));

    /* Isoceles triangle ABC */
    float angle_ABC = _inputParams.angle_ABC;
    float angle_BAC = (M_PI - angle_ABC) * 0.5f;
    float angle_BCA = angle_BAC;
    printf("Isoceles triangle ABC vertex angles:\n");
    printf("   angle_ABC             = %.3f degrees\n", GLKMathRadiansToDegrees(angle_ABC));
    printf("   angle_BAC = angle_BCA = %.3f degrees\n", GLKMathRadiansToDegrees(angle_BAC));

    /* Isoceles triangle AOC */
    float angle_AOC = (2.0f * asin(length_AC / (2.0f * length_OA)));
    float angle_OAC = (M_PI - angle_AOC) * 0.5f;
    float angle_OCA = angle_OAC;
    printf("Isoceles triangle AOC vertex angles:\n");
    printf("   angle_AOC             = %.3f degrees\n", GLKMathRadiansToDegrees(angle_AOC));
    printf("   angle_OAC = angle_OCA = %.3f degrees\n", GLKMathRadiansToDegrees(angle_OAC));

    /* Validate the tetrahedron via the 3D law of sines */
    {
        float epsilon = (1.0f / 1000.0f);
        /* O,ABC */
        {
            float lhs = (sin(angle_OAC) * sin(angle_OCB) * sin(angle_ABO));
            float rhs = (sin(angle_OCA) * sin(angle_CBO) * sin(angle_OAB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* A,OBC */
        {
            float lhs = (sin(angle_AOC) * sin(angle_BCA) * sin(angle_ABO));
            float rhs = (sin(angle_OCA) * sin(angle_ABC) * sin(angle_AOB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* B,AOC */
        {
            float lhs = (sin(angle_BAC) * sin(angle_OCB) * sin(angle_AOB));
            float rhs = (sin(angle_BCA) * sin(angle_COB) * sin(angle_OAB));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
        /* C,ABO */
        {
            float lhs = (sin(angle_OAC) * sin(angle_COB) * sin(angle_ABC));
            float rhs = (sin(angle_AOC) * sin(angle_CBO) * sin(angle_BAC));
            if (fabsf(lhs - rhs) > epsilon) {
                printf("ERROR: Invalid tetrahedron\n");
                return true;
            }
        }
    }
        
    /* Compute vertex positions for the first tetrahedron */
    float      length_AM = length_AC * 0.5f; /* M is at the midpoint between A and C */
    GLKVector3 A0 = GLKVector3Make(-length_AM, 0, 0);
    GLKVector3 C0 = GLKVector3Make(+length_AM, 0, 0);
    GLKVector3 B0 = GLKVector3Make(0, 0, sqrtf(sq(length_BA) - sq(length_AM)));
    GLKVector3 O;
    O.x = 0;
    O.z = ((sq(length_OB) - sq(length_OA) + sq(length_AM) - sq(B0.z)) / 
           (-2.0f * B0.z));
    O.y = sqrtf(sq(length_OA) - sq(length_AM) - sq(O.z));

    /* Translate first tetrahedron such that O.xz = 0 */
    A0.z -= O.z;
    B0.z -= O.z;
    C0.z -= O.z;
    O.z = 0;
    
    /* Second tetrahedron ABC is just first tetrahedron ABC with Z negated */
    GLKVector3 A1 = GLKVector3Multiply(A0, GLKVector3Make(1.0f, 1.0f, -1.0f));
    GLKVector3 B1 = GLKVector3Multiply(B0, GLKVector3Make(1.0f, 1.0f, -1.0f));
    GLKVector3 C1 = GLKVector3Multiply(C0, GLKVector3Make(1.0f, 1.0f, -1.0f));

    /* Output vertex coordinates */
    printf("Vertex coordinates:\n");
    printf("   O  = [%+.3f, %+.3f, %+.3f]\n", O.x, O.y, O.z);
    printf("   B0 = [%+.3f, %+.3f, %+.3f]\n", B0.x, B0.y, B0.z);
    printf("   A0 = [%+.3f, %+.3f, %+.3f]\n", A0.x, A0.y, A0.z);
    printf("   C0 = [%+.3f, %+.3f, %+.3f]\n", C0.x, C0.y, C0.z);
    printf("   B1 = [%+.3f, %+.3f, %+.3f]\n", B1.x, B1.y, B1.z);
    printf("   A1 = [%+.3f, %+.3f, %+.3f]\n", A1.x, A1.y, A1.z);
    printf("   C1 = [%+.3f, %+.3f, %+.3f]\n", C1.x, C1.y, C1.z);

    /* Overall structure stats */
    GLKVector2 footprint              = GLKVector2Make(C0.x - A0.x,
                                                       A1.z - A0.z);
    float      footprint_area         = (footprint.x * footprint.y);
    float      footprint_aspect_ratio = (footprint.x / footprint.y);

    float      height         = O.y;
    float      triangle_area  = GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(O, B0),
                                                                        GLKVector3Subtract(A0, B0))) * 0.5f;

    float      walkway_top_angle  = atanf(B1.z / O.y) * 2.0f;
    float      walkway_base_width = B1.z - B0.z;
    float      shoulder_height    = 5.0f;
    float      walkway_shoulder_width = ((O.y - shoulder_height) * B1.z * 2.0f) / O.y;

    printf("Structural shape summary:\n");
    printf("   Footprint dimensions   = [%.3f, %.3f] ft\n", footprint.x, footprint.y);
    printf("   Footprint surface area = %.3f ft^2\n", footprint_area);
    printf("   Footprint aspect ratio = %.3f\n", footprint_aspect_ratio);
    printf("   Height                 = %.3f ft\n", height);
    printf("   Triangle surface area  = %.3f ft^2\n", triangle_area);
    printf("   Walkway top angle      = %.3f degrees\n", GLKMathRadiansToDegrees(walkway_top_angle));
    printf("   Walkway base width     = %.3f ft\n", walkway_base_width);
    printf("   Walkway shoulder width = %.3f ft (at %.3f ft shoulder height)\n", walkway_shoulder_width, shoulder_height);

    /* Important dihedral angles */
    GLKVector3 BO = GLKVector3Subtract(B0, O);
    GLKVector3 BA = GLKVector3Subtract(B0, A0);
    GLKVector3 BC = GLKVector3Subtract(B0, C0);
    GLKVector3 norm_BOA = GLKVector3Normalize(GLKVector3CrossProduct(BO, BA));
    GLKVector3 norm_BOC = GLKVector3Normalize(GLKVector3CrossProduct(BO, BC));
    GLKVector3 norm_ABC = GLKVector3Make(0.0f, 1.0f, 0.0f);
    float      angle_BOA_BOC = acosf(GLKVector3DotProduct(norm_BOA, norm_BOC));
    float      angle_BOA_ABC = acosf(GLKVector3DotProduct(norm_BOA, norm_ABC));
    
    printf("Important dihedral angles:\n");
    printf("   Between triangle pairs (angle_BOA_BOC)      = %.3f degrees\n", GLKMathRadiansToDegrees(angle_BOA_BOC));
    printf("   Between triangle and ground (angle_BOA_ABC) = %.3f degrees\n", GLKMathRadiansToDegrees(angle_BOA_ABC));

    /* Frame info */
    float      frame_perimeter_length    = ((length_BA * 4) + 
                                            (length_OA * 4) + 
                                            (length_OB * 4));
    /* Shitty estimate on reinforcement.  Need to design first, but assuming 3 per triangle, so this is approx.
       Also, one cross-bar between B0 and B1 */
    float      frame_reinforce_length    = (length_BA * 1.6 * 4) + walkway_base_width;
    float      frame_total_length        = (frame_perimeter_length + frame_reinforce_length);
    GLKVector2 frame_xsection_dim        = GLKVector2Make(0.75f, 1.5f);
    float      frame_xsection_area       = (frame_xsection_dim.x * frame_xsection_dim.y);
    float      frame_wall_thickness      = (1.0f / 16.0f);
    GLKVector2 frame_xsection_inner_dim  = GLKVector2Make(frame_xsection_dim.x - (frame_wall_thickness * 2.0f),
                                                          frame_xsection_dim.y - (frame_wall_thickness * 2.0f));
    float      frame_xsection_inner_area = (frame_xsection_inner_dim.x * frame_xsection_inner_dim.y);
    float      frame_xsection_metal_area = (frame_xsection_area - frame_xsection_inner_area);
    float      frame_metal_volume        = (frame_xsection_metal_area * (frame_total_length * 3.0f * 4.0f));
    float      metal_density             = 0.289f; /* lb/in^3 */
    float      metal_cost                = 4.4f; /* $ / ft */
    float      frame_mass                = (frame_metal_volume * metal_density);
    float      frame_cost                = (frame_total_length * metal_cost);

    printf("Frame info:\n");
    printf("   Perimeter length         = %.3f ft\n", frame_perimeter_length);
    printf("   Reinforce length         = %.3f ft\n", frame_reinforce_length);
    printf("   Total length             = %.3f ft\n", frame_total_length);
    printf("   Cross-section dimensions = [%.3f, %.3f] in\n", frame_xsection_dim.x, frame_xsection_dim.y);
    printf("   Wall thickness           = %.3f in\n", frame_wall_thickness);
    printf("   Cross-section metal area = %.3f in^2\n", frame_xsection_metal_area);
    printf("   Metal volume             = %.3f in^3 (%.3f ft^3)\n", frame_metal_volume, frame_metal_volume / (12.0f * 12.0f * 12.0f));
    printf("   Metal density            = %.3f lb/in^3\n", metal_density);
    printf("   Mass                     = %.3f lb\n", frame_mass);
    printf("   Cost                     = $%.3f\n", frame_cost);
    
    /* Mirror assembly */
    float      mirror_surface_area       = (triangle_area * 8.0f);
    float      mirror_unit_cost          = (220.0f / (8.0f * 4.0)); /* $ / ft^2 */
    float      mirror_cost               = (mirror_surface_area * mirror_unit_cost);
    float      mirror_bolts_spacing      = 2.0f; /* ft */
#if 0
    float      mirror_bolt_count         = (8 * /* 4 triangles, requiring bolts to hold mirror on both sides */
                                            (3 + /* Bolt on each vertex */
                                             ((length_BA / mirror_bolts_spacing) - 1) + /* spaced out along BA */
                                             ((length_OA / mirror_bolts_spacing) - 1) + /* ... and OA          */
                                             ((length_OB / mirror_bolts_spacing) - 1)   /* ... and OB          */
                                             )
                                            );
#else
    float      mirror_bolt_count         = ((frame_total_length * 2) /  /* x 2 due to double sided mirror attachment */
                                            mirror_bolts_spacing);
#endif
    float      mirror_per_bolt_cost      = (3.67f / 10.0f); /* McMaster Part #90585A537
                                                               316 Stainless Steel Hex Drive Flat Head Screw, 82 Degree Countersink Angle, 1/4"-20 Thread Size, 1/2" Long */
    float      mirror_bolt_cost          = (mirror_bolt_count * mirror_per_bolt_cost);

    printf("Mirror coating info:\n");
    printf("   Total surface area       = %.3f ft^2\n", mirror_surface_area);
    printf("   Mirror cost              = $%.3f\n", mirror_cost);
    printf("   Mirror bolt count        = %f\n", mirror_bolt_count);
    printf("   Mirror bolt cost         = $%.3f\n", mirror_bolt_cost);

    /* Wind force calculations */
    /* Project triangle(OBA) onto XY plane */
    GLKVector3 ApXY = GLKVector3Multiply(A0, GLKVector3Make(1.0f, 1.0f, 0.0f));
    GLKVector3 BpXY = GLKVector3Multiply(B0, GLKVector3Make(1.0f, 1.0f, 0.0f));
    GLKVector3 OpXY = GLKVector3Multiply(O,  GLKVector3Make(1.0f, 1.0f, 0.0f));
    /* Compute projected surface area */
    float      triangle_surface_area_XY = (GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(BpXY, ApXY),
                                                                                   GLKVector3Subtract(BpXY, OpXY))) * 0.5f);
    float      total_surface_area_XY    = (triangle_surface_area_XY * 2.0f);
    /* Project triangle(OBA) onto YZ plane */
    GLKVector3 ApYZ = GLKVector3Multiply(A0, GLKVector3Make(0.0f, 1.0f, 1.0f));
    GLKVector3 BpYZ = GLKVector3Multiply(B0, GLKVector3Make(0.0f, 1.0f, 1.0f));
    GLKVector3 OpYZ = GLKVector3Multiply(O,  GLKVector3Make(0.0f, 1.0f, 1.0f));
    /* Compute projected surface area */
    float      triangle_surface_area_YZ = (GLKVector3Length(GLKVector3CrossProduct(GLKVector3Subtract(BpYZ, ApYZ),
                                                                                   GLKVector3Subtract(BpYZ, OpYZ))) * 0.5f);
    float      total_surface_area_YZ    = (triangle_surface_area_YZ * 2.0f);
    printf("Wind:\n");
    printf("   XY plane:\n");
    printf("      Total surface area = %.3f ft^2\n", total_surface_area_XY);
    for (float mph = 5.0f; mph <= 100.0f; mph += 5.0f) {
        float pressure   = sq(MPH_To_FTSec(mph)) * 0.00256; /* lb/ft^2 */
        float coeff_drag = 1.0f;
        float force      = (total_surface_area_XY * pressure * coeff_drag);
        printf("      Side force at %f MPH = %.0f lbs\n", mph, force);
    }
    printf("   YZ plane:\n");
    printf("      Total surface area = %.3f ft^2\n", total_surface_area_YZ);
    for (float mph = 5.0f; mph <= 100.0f; mph += 5.0f) {
        float pressure   = sq(MPH_To_FTSec(mph)) * 0.00256; /* lb/ft^2 */
        float coeff_drag = 1.0f;
        float force      = (total_surface_area_YZ * pressure * coeff_drag);
        printf("      Side force at %f MPH = %.0f lbs\n", mph, force);
    }
    
    return false;
}

int main(void)
{
    BM11Model model;

    if (model.evaluate()) {
        printf("ERROR: Model evaluation error\n");
    }

    return 0;
}


