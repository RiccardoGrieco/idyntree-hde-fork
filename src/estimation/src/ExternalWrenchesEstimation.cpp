/*
 * Copyright (C) 2016 Fondazione Istituto Italiano di Tecnologia
 * Author: Silvio Traversaro
 * email:  silvio.traversaro@iit.it
 * website: www.icub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <iDynTree/Estimation/ExternalWrenchesEstimation.h>

#include <iDynTree/Core/EigenHelpers.h>
#include <iDynTree/Core/EigenMathHelpers.h>
#include <iDynTree/Core/SpatialMomentum.h>

#include <iDynTree/Model/Model.h>
#include <iDynTree/Model/Traversal.h>
#include <iDynTree/Model/SubModel.h>
#include <iDynTree/Model/LinkState.h>
#include <iDynTree/Model/JointState.h>
#include <iDynTree/Model/ContactWrench.h>

#include <iDynTree/Sensors/Sensors.h>
#include <iDynTree/Sensors/SixAxisFTSensor.h>

namespace iDynTree
{

LinkUnknownWrenchContacts::LinkUnknownWrenchContacts(unsigned int nrOfLinks)
{
    this->resize(nrOfLinks);
}


LinkUnknownWrenchContacts::LinkUnknownWrenchContacts(const iDynTree::Model& model)
{
    this->resize(model);
}

void LinkUnknownWrenchContacts::resize(const iDynTree::Model& model)
{
    resize(model.getNrOfLinks());
}

void LinkUnknownWrenchContacts::resize(unsigned int nrOfLinks)
{
    // To avoid dynamic memory allocation at runtime
    // we make sure that the vector have at least spaces for 3 contacts
    const size_t reservedSlots = 3;
    m_linkUnknownWrenchContacts.resize(nrOfLinks);
    for(size_t l=0; l < nrOfLinks; l++)
    {
        m_linkUnknownWrenchContacts[l].resize(0);
        m_linkUnknownWrenchContacts[l].reserve(reservedSlots);
    }
}

const UnknownWrenchContact& LinkUnknownWrenchContacts::contactWrench(const LinkIndex linkIndex, const size_t contactIndex) const
{
    return m_linkUnknownWrenchContacts[linkIndex][contactIndex];
}

UnknownWrenchContact& LinkUnknownWrenchContacts::contactWrench(const LinkIndex linkIndex, const size_t contactIndex)
{
    return m_linkUnknownWrenchContacts[linkIndex][contactIndex];
}

size_t LinkUnknownWrenchContacts::getNrOfContactsForLink(const LinkIndex linkIndex) const
{
    return m_linkUnknownWrenchContacts[linkIndex].size();
}

void LinkUnknownWrenchContacts::setNrOfContactsForLink(const LinkIndex linkIndex, const size_t nrOfContacts)
{
    m_linkUnknownWrenchContacts[linkIndex].resize(nrOfContacts);
    return;
}

void estimateExternalWrenchesBuffers::resize(const SubModelDecomposition& subModels)
{
    this->resize(subModels.getNrOfSubModels(),subModels.getNrOfLinks());
}

void estimateExternalWrenchesBuffers::resize(const size_t nrOfSubModels, const size_t nrOfLinks)
{
    A.resize(nrOfSubModels);
    x.resize(nrOfSubModels);
    b.resize(nrOfSubModels);
    pinvA.resize(nrOfSubModels);

    b_contacts_subtree.resize(nrOfLinks);

    subModelBase_H_link.resize(nrOfLinks);
}

bool estimateExternalWrenchesBuffers::isConsistent(const SubModelDecomposition& subModels) const
{
    return (subModels.getNrOfSubModels() == A.size()) &&
           (b_contacts_subtree.getNrOfLinks() == subModels.getNrOfLinks());
}

Wrench getMeasuredWrench(const SensorsList & sensors_list,
                         const SensorsMeasurements & sensor_measures,
                         const LinkIndex link_id)
{

    size_t NrOfFTSensors = sensors_list.getNrOfSensors(SIX_AXIS_FORCE_TORQUE);
    ::iDynTree::Wrench total_measured_applied_wrench = ::iDynTree::Wrench::Zero();
    for(size_t ft=0; ft < NrOfFTSensors; ft++ )
    {
        ::iDynTree::SixAxisForceTorqueSensor * sens
            = (::iDynTree::SixAxisForceTorqueSensor *) sensors_list.getSensor(::iDynTree::SIX_AXIS_FORCE_TORQUE,ft);

        assert(sens != 0);

        Wrench measured_wrench_on_link = Wrench::Zero();
        Wrench measured_wrench_by_sensor;

        bool ok = sensor_measures.getMeasurement(SIX_AXIS_FORCE_TORQUE,ft,measured_wrench_by_sensor);

        assert(ok);

        // If the sensor with index ft is not attached to the link
        // this function return a zero wrench
        sens->getWrenchAppliedOnLink(link_id,measured_wrench_by_sensor,measured_wrench_on_link);

        //Sum the given wrench to the return value
        total_measured_applied_wrench = total_measured_applied_wrench+measured_wrench_on_link;
    }

    return total_measured_applied_wrench;
}

bool estimateExternalWrenches(const Model& model,
                              const SubModelDecomposition& subModels,
                              const SensorsList& sensors,
                              const LinkUnknownWrenchContacts& unknownWrenches,
                              const JointDoubleArray & jointPos,
                              const LinkVelArray& linkVel,
                              const LinkAccArray& linkProperAcc,
                              const SensorsMeasurements& ftSensorsMeasurements,
                                    estimateExternalWrenchesBuffers& bufs,
                                    LinkContactWrenches& outputContactWrenches)
{
    /**< value extracted from old iDynContact */
    double tol = 1e-7;

    // Resize the output data structure
    outputContactWrenches.resize(model);

    // Solve the problem for each submodel
    for(size_t sm=0; sm < subModels.getNrOfSubModels(); sm++ )
    {
        // Number of unknowns for this submodel
        int unknowns = 0;
        const Traversal & subModelTraversal = subModels.getTraversal(sm);
        // First compute the known term of the estimation for each link:
        // this loop is similar to the dynamic phase of the RNEA
        // \todo pimp up performance as done in RNEADynamicPhase
        for(int traversalEl = subModelTraversal.getNrOfVisitedLinks()-1; traversalEl >= 0; traversalEl--)
        {
            LinkConstPtr visitedLink = subModelTraversal.getLink(traversalEl);
            LinkIndex    visitedLinkIndex = visitedLink->getIndex();
            LinkConstPtr parentLink  = subModelTraversal.getParentLink(traversalEl);

            const iDynTree::SpatialInertia & I = visitedLink->getInertia();
            const iDynTree::SpatialAcc     & properAcc = linkProperAcc(visitedLinkIndex);
            const iDynTree::Twist          & v = linkVel(visitedLinkIndex);
            bufs.b_contacts_subtree(visitedLinkIndex) = I*properAcc + v*(I*v) - getMeasuredWrench(sensors,ftSensorsMeasurements,visitedLinkIndex);

            // Iterate on childs of visitedLink
            // We obtain all the children as all the neighbors of the link, except
            // for its parent
            // \todo TODO this point is definitly Tree-specific
            // \todo TODO this "get child" for is duplicated in the code, we
            //            should try to consolidate it
            for(unsigned int neigh_i=0; neigh_i < model.getNrOfNeighbors(visitedLinkIndex); neigh_i++)
            {
                LinkIndex neighborIndex = model.getNeighbor(visitedLinkIndex,neigh_i).neighborLink;
                if( !parentLink || neighborIndex != parentLink->getIndex() )
                {
                    LinkIndex childIndex = neighborIndex;
                    IJointConstPtr neighborJoint = model.getJoint(model.getNeighbor(visitedLinkIndex,neigh_i).neighborJoint);
                    Transform visitedLink_X_child = neighborJoint->getTransform(jointPos,visitedLinkIndex,childIndex);

                    // One term of the sum in Equation 5.20 in Featherstone 2008
                    bufs.b_contacts_subtree(visitedLinkIndex) = bufs.b_contacts_subtree(visitedLinkIndex)
                                                               + visitedLink_X_child*bufs.b_contacts_subtree(childIndex);
                }
            }

            if( parentLink == 0 )
            {
                // If the visited link is the base of the submodel, the
                // compute known terms is the known term of the submodel itself
                toEigen(bufs.b[sm]).segment<3>(0) = toEigen(bufs.b_contacts_subtree(visitedLinkIndex).getLinearVec3());
                toEigen(bufs.b[sm]).segment<3>(3) = toEigen(bufs.b_contacts_subtree(visitedLinkIndex).getAngularVec3());
            }

            // While we are in this loop, we also keep track of the number of unknowns
            for(size_t contact = 0;
                contact < unknownWrenches.getNrOfContactsForLink(visitedLinkIndex);
                contact++ )
            {
                const UnknownWrenchContact & unknownWrench = unknownWrenches.contactWrench(visitedLinkIndex,contact);

                switch( unknownWrench.unknownType )
                {
                    case FULL_WRENCH:
                        unknowns += 6;
                        break;
                    case PURE_FORCE:
                        unknowns += 3;
                        break;
                    case PURE_FORCE_WITH_KNOWN_DIRECTION:
                        unknowns += 1;
                        break;
                    default:
                        break;
                }
            }

        }

        // Now we compute the A matrix
        bufs.A[sm].resize(6,unknowns);
        bufs.x[sm].resize(unknowns);
        bufs.pinvA[sm].resize(unknowns,6);

        // As a first step, we need to compute the transform between each link and the base
        // of its submodel (we are computing the estimation equation in the submodel base frame
        computeTransformToSubModelBase(model,subModels,jointPos,bufs.subModelBase_H_link);

        Eigen::Map< Eigen::Matrix<double,Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > Aeig = toEigen(bufs.A[sm]);
        int AcolsToWrite = 0;
        // We then need to compute the different submatrix of A for each type of unknown
        for(int traversalEl = (int)subModelTraversal.getNrOfVisitedLinks()-1; traversalEl >= 0; traversalEl--)
        {
            LinkConstPtr visitedLink = subModelTraversal.getLink(traversalEl);
            LinkIndex    visitedLinkIndex = visitedLink->getIndex();

            size_t nrOfContactForLink = unknownWrenches.getNrOfContactsForLink(visitedLinkIndex);

            for(size_t contact = 0;
                contact < nrOfContactForLink;
                contact++ )
            {
                const UnknownWrenchContact & unknownWrench = unknownWrenches.contactWrench(visitedLinkIndex,contact);

                // We are estimate the force and the torque at the contact point,
                // so we need to compute the transform between a frame with the orientation
                // of the link frame and the origin given by the contact point
                const Transform & subModelBase_H_link = bufs.subModelBase_H_link(visitedLinkIndex);
                Transform link_H_contact = Transform(Rotation::Identity(),unknownWrench.contactPoint);
                Transform subModelBase_H_contact = subModelBase_H_link*link_H_contact;

                switch( unknownWrench.unknownType )
                {
                    case FULL_WRENCH:
                        Aeig.block<6,6>(0,AcolsToWrite) = toEigen(subModelBase_H_contact.asAdjointTransformWrench());
                        AcolsToWrite += 6;
                        break;
                    case PURE_FORCE:
                        Aeig.block<6,3>(0,AcolsToWrite) =
                            toEigen(subModelBase_H_contact.asAdjointTransformWrench()).block<6,3>(0,0);
                        AcolsToWrite += 3;
                        break;
                    case PURE_FORCE_WITH_KNOWN_DIRECTION:
                    {
                        Eigen::Matrix<double,6,1> fDir;
                        fDir.setZero();
                        fDir.segment<3>(0) = toEigen(unknownWrench.forceDirection);
                        Aeig.block<6,1>(0,AcolsToWrite) =
                            toEigen(subModelBase_H_contact.asAdjointTransformWrench())*fDir;
                        AcolsToWrite += 1;
                    }
                        break;
                    default:
                        break;
                }
            }
        }

        // Now we compute the pseudo inverse
        pseudoInverse(toEigen(bufs.A[sm]),
                      toEigen(bufs.pinvA[sm]),
                      tol);

        // Now we compute the unknowns
        toEigen(bufs.x[sm]) = toEigen(bufs.pinvA[sm])*toEigen(bufs.b[sm]);

        // We copy the estimated unknowns in the outputContactWrenches
        // Note that the logic of conversion between input/output contacts should be
        // the same used before
        size_t nextUnknownToRead = 0;
        for(int traversalEl = subModelTraversal.getNrOfVisitedLinks()-1; traversalEl >= 0; traversalEl--)
        {
            LinkConstPtr visitedLink = subModelTraversal.getLink(traversalEl);
            LinkIndex    visitedLinkIndex = visitedLink->getIndex();

            size_t nrOfContactForLink = unknownWrenches.getNrOfContactsForLink(visitedLinkIndex);

            outputContactWrenches.setNrOfContactsForLink(visitedLinkIndex,nrOfContactForLink);

            for(size_t contact = 0;
                contact < nrOfContactForLink;
                contact++ )
            {
                const UnknownWrenchContact & unknownWrench = unknownWrenches.contactWrench(visitedLinkIndex,contact);
                Wrench & estimatedWrench = outputContactWrenches.contactWrench(visitedLinkIndex,contact).contactWrench();
                outputContactWrenches.contactWrench(visitedLinkIndex,contact).contactPoint() = unknownWrench.contactPoint;

                switch( unknownWrench.unknownType )
                {
                    case FULL_WRENCH:
                        estimatedWrench.getLinearVec3()(0) = bufs.x[sm](nextUnknownToRead+0);
                        estimatedWrench.getLinearVec3()(1) = bufs.x[sm](nextUnknownToRead+1);
                        estimatedWrench.getLinearVec3()(2) = bufs.x[sm](nextUnknownToRead+2);
                        estimatedWrench.getAngularVec3()(0) = bufs.x[sm](nextUnknownToRead+3);
                        estimatedWrench.getAngularVec3()(1) = bufs.x[sm](nextUnknownToRead+4);
                        estimatedWrench.getAngularVec3()(2) = bufs.x[sm](nextUnknownToRead+5);
                        nextUnknownToRead += 6;
                        break;
                    case PURE_FORCE:
                        estimatedWrench.getLinearVec3()(0) = bufs.x[sm](nextUnknownToRead+0);
                        estimatedWrench.getLinearVec3()(1) = bufs.x[sm](nextUnknownToRead+1);
                        estimatedWrench.getLinearVec3()(2) = bufs.x[sm](nextUnknownToRead+2);
                        estimatedWrench.getAngularVec3().zero();
                        nextUnknownToRead += 3;
                        break;
                    case PURE_FORCE_WITH_KNOWN_DIRECTION:
                        toEigen(estimatedWrench.getLinearVec3()) =
                            bufs.x[sm](nextUnknownToRead+0)*toEigen(unknownWrench.forceDirection);
                        estimatedWrench.getAngularVec3().zero();
                        nextUnknownToRead += 1;
                        break;
                    default:
                        break;
                }
            }
        }

    }

    return true;
}



}

