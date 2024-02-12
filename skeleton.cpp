#include <fstream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "skeleton.h"
#include "transform.h"

#ifdef WIN32
	#pragma warning(disable : 4996)
#endif

int Skeleton::numBonesInSkel(Bone bone){
    //this is just a counter
    Bone* tmp = bone.sibling;
    int numBones = 0;
    while (tmp != NULL)
    {
        if (tmp->child != NULL)
            numBones += numBonesInSkel(*(tmp->child));
        numBones++;
        tmp = tmp->sibling;
    }
    if (bone.child != NULL)
        return numBones + 1 + numBonesInSkel(*bone.child);
    else
        return numBones + 1;
}

void Skeleton::removeChar(char* str){
    //this is for the file to get rid of uneccesary final characters
    //maybe just implement in the readASF file function instead of a whole function
    if (str[strlen(str) - 1] == '\r')
        str[strlen(str) - 1] = 0;
}

int Skeleton::movBonesInSkel(Bone bone){
    //this is very similar to numBonesInSkel, just working with the dof
    Bone* tmp = bone.sibling;
    int numBones = 0;

    if (bone.dof > 0) numBones++;

    while (tmp != NULL)
    {
        if (tmp->child != NULL)
            numBones += movBonesInSkel(*(tmp->child));
        if (tmp->dof > 0)
            numBones++;
        tmp = tmp->sibling;
    }

    if (bone.child != NULL)
        return (numBones + movBonesInSkel(*bone.child));
    else
        return numBones;
}

int Skeleton::name2idx(char *name){
    //helper function to convert ASF part name into bone index
    int i = 0;
    while (strcmp(m_pBoneList[i].name, name) != 0 && i++ < NUM_BONES_IN_ASF_FILE);
    return m_pBoneList[i].idx;
}

char * Skeleton::idx2name(int idx){
    //not sure what this does, find it in the main file for its use
    //it probably converts backwards
    int i = 0;
    while (m_pBoneList[i].idx != idx && i++ < NUM_BONES_IN_ASF_FILE);
    return m_pBoneList[i].name;
}

//main function come back to it
int Skeleton::readASFfile(char* asf_filename, double scale) {
    //check file exists and open it
    std::ifstream is(asf_filename, std::ios::in);
    if (is.fail()) return -1;
    //skip through header
    char str[2048], keyword[256];
    while (1) {
        is.getline(str, 2048);
        removeChar(str);
        sscanf(str, "%s", keyword);
        if (strcmp(keyword, ":bonedata") == 0)
            break;
    }

    //read bone information; orintation, translation, dof
    is.getline(str, 2048);
    removeChar(str);
    char	part[256], * token;
    double length;

    bool done = false;
    for (int i = 1; (!done) && (i < MAX_BONES_IN_ASF_FILE); i++) {
        m_pBoneList[i].dof = 0;
        m_pBoneList[i].dofrx = m_pBoneList[i].dofry = m_pBoneList[i].dofrz = 0;
        m_pBoneList[i].doftx = m_pBoneList[i].dofty = m_pBoneList[i].doftz = 0;
        m_pBoneList[i].doftl = 0;
        m_pBoneList[i].sibling = NULL;
        m_pBoneList[i].child = NULL;
        NUM_BONES_IN_ASF_FILE++;
        MOV_BONES_IN_ASF_FILE++;

        while (1) {
            is.getline(str, 2048);
            removeChar(str);
            sscanf(str, "%s", keyword);

            if (strcmp(keyword, "end") == 0) break;
            if (strcmp(keyword, ":hierarchy") == 0) {
                MOV_BONES_IN_ASF_FILE -= 1;
                NUM_BONES_IN_ASF_FILE -= 1;
                done = true;
                break;
            }

            //id of bone
            if (strcmp(keyword, "id") == 0)
                m_pBoneList[i].idx = NUM_BONES_IN_ASF_FILE - 1;

            //name of bone
            if (strcmp(keyword, "name") == 0) {
                sscanf(str, "%s %s", keyword, part);
                sscanf(str, "%s %s", keyword, m_pBoneList[i].name);
            }

            //direction vector in global coordinates
            if (strcmp(keyword, "direction") == 0)
                sscanf(str, "%s %lf %lf %lf", keyword, &m_pBoneList[i].dir[0], &m_pBoneList[i].dir[1], &m_pBoneList[i].dir[2]);

            //length of bone
            if (strcmp(keyword, "length") == 0)
                sscanf(str, "%s %lf", keyword, &length);

            //orientation of bone's local coordinate system relative to the world coordnate system
            if (strcmp(keyword, "axis") == 0)
                sscanf(str, "%s %lf %lf %lf", keyword, &m_pBoneList[i].axis_x, &m_pBoneList[i].axis_y, &m_pBoneList[i].axis_z);

            //dof of bone
            if (strcmp(keyword, "dof") == 0) {
                token = strtok(str, " ");
                m_pBoneList[i].dof = 0;
                while (token != NULL) {
                    int tdof = m_pBoneList[i].dof;

                    if (strcmp(token, "rx") == 0) { m_pBoneList[i].dofrx = 1; m_pBoneList[i].dofo[tdof] = 1; }
                    else if (strcmp(token, "ry") == 0) { m_pBoneList[i].dofry = 1; m_pBoneList[i].dofo[tdof] = 2; }
                    else if (strcmp(token, "rz") == 0) { m_pBoneList[i].dofrz = 1; m_pBoneList[i].dofo[tdof] = 3; }
                    else if (strcmp(token, "tx") == 0) { m_pBoneList[i].doftx = 1; m_pBoneList[i].dofo[tdof] = 4; }
                    else if (strcmp(token, "ty") == 0) { m_pBoneList[i].dofty = 1; m_pBoneList[i].dofo[tdof] = 5; }
                    else if (strcmp(token, "tz") == 0) { m_pBoneList[i].doftz = 1; m_pBoneList[i].dofo[tdof] = 6; }
                    else if (strcmp(token, "l") == 0) { m_pBoneList[i].doftl = 1; m_pBoneList[i].dofo[tdof] = 7; }
                    else if (strcmp(token, "dof") == 0) { goto end; }
                    else { printf("UNKNOWN %s\n", token); }

                    m_pBoneList[i].dof++;
                    m_pBoneList[i].dofo[m_pBoneList[i].dof] = 0;
                    end:
                    token = strtok(NULL, " ");
                }
                //debug
                printf("Bone %d DOF: ", i);
                for (int x = 0; (x < 7) && (m_pBoneList[i].dofo[x] != 0); x++)
                    printf("%d ", m_pBoneList[i].dofo[x]);
                printf("\n");
            }
        }

        //store the data to the data structure
        //m_pBoneList[i].idx = name2idx(part); (not sure what this is meant to do yet)
        if ((!m_pBoneList[i].dofrx) && (!m_pBoneList[i].dofry) && (!m_pBoneList[i].dofrz))
            MOV_BONES_IN_ASF_FILE -= 1;
        m_pBoneList[i].length = length * scale;
    }
    //debug
    printf("READ %d\n", NUM_BONES_IN_ASF_FILE);

    //read and build the skeleton hierarchy
    char* part_name;
    int j, parent = 0;

    //skip the begin line
    is.getline(str, 2048);
    removeChar(str);

    //Assign parent/child/sibling relatiionships
    while (1) {
        //read next line
        is.getline(str, 2048);
        removeChar(str);

        sscanf(str, "%s", keyword);

        //check if we are done
        if (strcmp(keyword, "end") == 0) break;
        else {
            //parse this line, it contains parent followed by children
            part_name = strtok(str, " ");
            j = 0;
            while (part_name != NULL) {
                if (j == 0) parent = name2idx(part_name);
                else setChildrenAndSibling(parent, &m_pBoneList[name2idx(part_name)]);
                part_name = strtok(NULL, " ");
                j++;
            }
        }
    }

    is.close();

    return 0;
}

Bone* Skeleton::getBone(Bone* ptr, int bIndex) {
    //traverses the skeleton hierarchy and returns a pointer to the bone with index bIndex
    //ptr is to the root node when first called
    static Bone* theptr;
    if (ptr == NULL)
        return(NULL);
    else if (ptr->idx == bIndex){
        theptr = ptr;
        return(theptr);
    } else {
        getBone(ptr->child, bIndex);
        getBone(ptr->sibling, bIndex);
        return(theptr);
    }
}

int Skeleton::setChildrenAndSibling(int parent, Bone *pChild){
    //if a parent bone does not have a child, then pChild is set at parent's child
    //else pChild is set as a sibling of a parents existing child
    Bone* pParent;

    //Get pointer to root bone
    pParent = getBone(m_pRootBone, parent);

    if (pParent == NULL) {
        printf("inbord bone is undefined\n");
        return(0);
    } else {
        //if pParent bone does not have a child
        //set pChild as parent bone child
        if (pParent->child == NULL) {
            pParent->child = pChild;
        } else {
            //if pParent bone already has a child 
            //set pChils as pParent bone's child sibling
            pParent = pParent->child;
            while (pParent->sibling != NULL)
                pParent = pParent->sibling;

            pParent->sibling = pChild;
        }
        return(1);
    }
}

Bone * Skeleton::getRoot(){
    //simple getter
    return(m_pRootBone);
}

//The following 3 functions compute relative orientation and 
//translation between parent and child bones (in the local coordinate of the parent body)
void Skeleton::compute_rotation_parent_child(Bone *parent, Bone *child){
    //rotation from this bone local coordinate system to the coordinate system of its parent
    double Rx[4][4], Ry[4][4], Rz[4][4], tmp[4][4], tmp1[4][4], tmp2[4][4];

    if (child != NULL) {
        rotationZ(Rz, -parent->axis_z);
        rotationY(Ry, -parent->axis_y);
        rotationX(Rx, -parent->axis_x);
        matrix_mult(Rx, Ry, tmp);
        matrix_mult(tmp, Rz, tmp1);

        rotationZ(Rz, child->axis_z);
        rotationY(Ry, child->axis_y);
        rotationX(Rx, child->axis_x);
        matrix_mult(Rz, Ry, tmp);
        matrix_mult(tmp, Rx, tmp2);

        matrix_mult(tmp1, tmp2, tmp);
        matrix_transpose(tmp, child->rot_parent_current);
    }
}

void Skeleton::ComputeRotationToParentCoordSystem(Bone *bone){
    //loop through all bones to calculate local coordinate's direction vector and relative orientation
    //check that this and the above are used at separate times (see if you can combine)
    int i;
    double Rx[4][4], Ry[4][4], Rz[4][4], tmp[4][4], tmp2[4][4];

    //Compute rot_parent_current for the root 

    //Compute tmp2, a matrix containing root 
    //joint local coordinate system orientation
    int root = Skeleton::getRootIndex();
    rotationZ(Rz, bone[root].axis_z);
    rotationY(Ry, bone[root].axis_y);
    rotationX(Rx, bone[root].axis_x);
    matrix_mult(Rz, Ry, tmp);
    matrix_mult(tmp, Rx, tmp2);
    //set bone[root].rot_parent_current to transpose of tmp2
    matrix_transpose(tmp2, bone[root].rot_parent_current);

    //Compute rot_parent_current for all other bones
    int numbones = numBonesInSkel(bone[0]);
    for (i = 0; i < numbones; i++) {
        if (bone[i].child != NULL) {
            compute_rotation_parent_child(&bone[i], bone[i].child);

            // compute parent child siblings...
            Bone* tmp = NULL;
            if (bone[i].child != NULL) tmp = (bone[i].child)->sibling;
            while (tmp != NULL) {
                compute_rotation_parent_child(&bone[i], tmp);
                tmp = tmp->sibling;
            }
        }
    }
}

void Skeleton::RotateBoneDirToLocalCoordSystem(){
    //transforms the direction vector to local coordinate
    int i;

    for (i = 1; i < NUM_BONES_IN_ASF_FILE; i++) {
        //Transform dir vector into local coordinate system
        rotationXYZ(&m_pBoneList[i].dir[0], -m_pBoneList[i].axis_x, -m_pBoneList[i].axis_y, -m_pBoneList[i].axis_z);
    }
}

//these functions set the pose of the skeleton
void Skeleton::setBasePosture(){
    //root at (0,0,0) and rotations set to 0
    int i;
    m_RootPos[0] = m_RootPos[1] = m_RootPos[2] = 0.0;

    for (i = 0; i < NUM_BONES_IN_ASF_FILE; i++)
    {
        m_pBoneList[i].rx = m_pBoneList[i].ry = m_pBoneList[i].rz = 0.0;
        m_pBoneList[i].tx = m_pBoneList[i].ty = m_pBoneList[i].tz = 0.0;
    }
}

void Skeleton::enableAllRotationalDOFs(){
    //i think this goes through counting degrees of freedom
    for (int j = 0; j < NUM_BONES_IN_ASF_FILE; j++)
    {
        if (m_pBoneList[j].dof == 0)
            continue;

        if (!m_pBoneList[j].dofrx)
        {
            m_pBoneList[j].dofrx = 1;
            m_pBoneList[j].rx = 0.0;
            m_pBoneList[j].dof++;
            m_pBoneList[j].dofo[m_pBoneList[j].dof - 1] = 1;
            m_pBoneList[j].dofo[m_pBoneList[j].dof] = 0;
        }

        if (!m_pBoneList[j].dofry)
        {
            m_pBoneList[j].dofry = 1;
            m_pBoneList[j].ry = 0.0;
            m_pBoneList[j].dof++;
            m_pBoneList[j].dofo[m_pBoneList[j].dof - 1] = 2;
            m_pBoneList[j].dofo[m_pBoneList[j].dof] = 0;
        }

        if (!m_pBoneList[j].dofrz)
        {
            m_pBoneList[j].dofrz = 1;
            m_pBoneList[j].rz = 0.0;
            m_pBoneList[j].dof++;
            m_pBoneList[j].dofo[m_pBoneList[j].dof - 1] = 3;
            m_pBoneList[j].dofo[m_pBoneList[j].dof] = 0;
        }
    }
}

void Skeleton::setPosture(Posture posture){
    //reset the pose based on the given posture

    //reset the root first
    m_RootPos[0] = posture.root_pos.p[0];
    m_RootPos[1] = posture.root_pos.p[1];
    m_RootPos[2] = posture.root_pos.p[2];

    //go through every bone from the root out rotating and translating as necessary
    for (int j = 0; j < NUM_BONES_IN_ASF_FILE; j++)
    {
        // if the bone has rotational degree of freedom in x direction
        if (m_pBoneList[j].dofrx)
            m_pBoneList[j].rx = posture.bone_rotation[j].p[0];

        if (m_pBoneList[j].doftx)
            m_pBoneList[j].tx = posture.bone_translation[j].p[0];

        // if the bone has rotational degree of freedom in y direction
        if (m_pBoneList[j].dofry)
            m_pBoneList[j].ry = posture.bone_rotation[j].p[1];

        if (m_pBoneList[j].dofty)
            m_pBoneList[j].ty = posture.bone_translation[j].p[1];

        // if the bone has rotational degree of freedom in z direction
        if (m_pBoneList[j].dofrz)
            m_pBoneList[j].rz = posture.bone_rotation[j].p[2];

        if (m_pBoneList[j].doftz)
            m_pBoneList[j].tz = posture.bone_translation[j].p[2];

        //find out what this references later
        if (m_pBoneList[j].doftl)
            m_pBoneList[j].tl = posture.bone_length[j].p[0];
    }
}

void Skeleton::set_bone_shape(Bone *bone){
    //setting the aspect ratio of each bone
    //see what is looks like when you change this?
    int root = Skeleton::getRootIndex();
    bone[root].aspx = 1;
    bone[root].aspy = 1;
    printf("READ %d\n", numBonesInSkel(bone[0]));
    printf("MOV %d\n", movBonesInSkel(bone[0]));
    int numbones = numBonesInSkel(bone[0]);
    for (int j = 1; j < numbones; j++)
    {
        bone[j].aspx = 0.25;
        bone[j].aspy = 0.25;
    }
}

Skeleton::Skeleton(char *asf_filename, double scale){
    //the constructor
    sscanf("root", "%s", m_pBoneList[0].name);
    NUM_BONES_IN_ASF_FILE = 1;
    MOV_BONES_IN_ASF_FILE = 1;
    m_pBoneList[0].dofo[0] = 4;
    m_pBoneList[0].dofo[1] = 5;
    m_pBoneList[0].dofo[2] = 6;
    m_pBoneList[0].dofo[3] = 1;
    m_pBoneList[0].dofo[4] = 2;
    m_pBoneList[0].dofo[5] = 3;
    m_pBoneList[0].dofo[6] = 0;
    //Initialization
    m_pBoneList[0].idx = getRootIndex();   // root of hierarchy
    m_pRootBone = &m_pBoneList[0];
    m_pBoneList[0].sibling = NULL;
    m_pBoneList[0].child = NULL;
    //you changed the next two lines, see if it still works
    m_pBoneList[0].dir[0] = m_pBoneList[0].dir[1] =  m_pBoneList[0].dir[2] = 0.;
    m_pBoneList[0].axis_x = m_pBoneList[0].axis_y = m_pBoneList[0].axis_z = 0.;
    m_pBoneList[0].length = 0.05;
    m_pBoneList[0].dof = 6;
    m_pBoneList[0].dofrx = m_pBoneList[0].dofry = m_pBoneList[0].dofrz = 1;
    m_pBoneList[0].doftx = m_pBoneList[0].dofty = m_pBoneList[0].doftz = 1;
    m_pBoneList[0].doftl = 0;
    m_RootPos[0] = m_RootPos[1] = m_RootPos[2] = 0;
    //	m_NumDOFs=6;
    tx = ty = tz = rx = ry = rz = 0.0;
    // build hierarchy and read in each bone's DOF information
    int code = readASFfile(asf_filename, scale);
    if (code != 0)
        throw 1;

    //transform the direction vector for each bone from the world coordinate system 
    //to it's local coordinate system
    RotateBoneDirToLocalCoordSystem();

    //Calculate rotation from each bone local coordinate system to the coordinate system of its parent
    //store it in rot_parent_current variable for each bone
    ComputeRotationToParentCoordSystem(m_pRootBone);

    //Set the aspect ratio of each bone 
    set_bone_shape(m_pRootBone);
}

Skeleton::~Skeleton(){}

void Skeleton::GetRootPosGlobal(double rootPosGlobal[3]){
    //simple getter
    rootPosGlobal[0] = m_RootPos[0];
    rootPosGlobal[1] = m_RootPos[1];
    rootPosGlobal[2] = m_RootPos[2];
}

void Skeleton::GetTranslation(double translation[3]){
    //simple getter
    translation[0] = tx;
    translation[1] = ty;
    translation[2] = tz;
}

void Skeleton::GetRotationAngle(double rotationAngle[3]){
    //simple getter
    rotationAngle[0] = rx;
    rotationAngle[1] = ry;
    rotationAngle[2] = rz;
}
