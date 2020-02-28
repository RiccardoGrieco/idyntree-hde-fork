/*
 * Copyright (C) 2016 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef IDYNTREE_BERDY_SPARSEMAPSOLVER_H
#define IDYNTREE_BERDY_SPARSEMAPSOLVER_H

#include <iDynTree/Core/Utils.h>
#include <iDynTree/Core/Transform.h>
#include <iDynTree/Core/VectorFixSize.h>
#include <iDynTree/Model/Indices.h>
#include <iDynTree/Core/VectorDynSize.h>

namespace iDynTree {

    class BerdyHelper;
    class VectorDynSize;
    class JointPosDoubleArray;
    class JointDOFsDoubleArray;

    template <iDynTree::MatrixStorageOrdering ordering>
    class SparseMatrix;


    /**
     * @warning This class is still in active development, and so API interface can change between iDynTree versions.
     * \ingroup iDynTreeExperimental
     */
    class BerdySparseMAPSolver
    {

        class BerdySparseMAPSolverPimpl;
        BerdySparseMAPSolverPimpl* m_pimpl;

    public:
        BerdySparseMAPSolver(BerdyHelper& berdyHelper);
        ~BerdySparseMAPSolver();

        void setDynamicsConstraintsPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor> & covariance);
        void setDynamicsRegularizationPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& covariance);
        void setDynamicsRegularizationPriorExpectedValue(const iDynTree::VectorDynSize& expectedValue);
        void setMeasurementsPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& covariance);

        // New methods with additional bool flag for task1
        void setDynamicsConstraintsPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor> & covariance, const bool& task1);
        void setDynamicsRegularizationPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& covariance, const bool& task1);
        void setDynamicsRegularizationPriorExpectedValue(const iDynTree::VectorDynSize& expectedValue, const bool& task1);
        void setMeasurementsPriorCovariance(const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& covariance, const bool& task1);

        const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& dynamicsConstraintsPriorCovarianceInverse() const; // Sigma_D^-1
        iDynTree::SparseMatrix<iDynTree::ColumnMajor>& dynamicsConstraintsPriorCovarianceInverse(); // Sigma_D^-1
        const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& dynamicsRegularizationPriorCovarianceInverse() const; // Sigma_d^-1
        iDynTree::SparseMatrix<iDynTree::ColumnMajor>& dynamicsRegularizationPriorCovarianceInverse(); // Sigma_d^-1
        const iDynTree::VectorDynSize& dynamicsRegularizationPriorExpectedValue() const; // mu_d
        iDynTree::VectorDynSize& dynamicsRegularizationPriorExpectedValue(); // mu_d
        const iDynTree::SparseMatrix<iDynTree::ColumnMajor>& measurementsPriorCovarianceInverse() const; // Sigma_y^-1
        iDynTree::SparseMatrix<iDynTree::ColumnMajor>& measurementsPriorCovarianceInverse(); // Sigma_y^-1

        bool isValid();

        bool initialize();

        void updateEstimateInformationFixedBase(const JointPosDoubleArray& jointsConfiguration,
                                                const JointDOFsDoubleArray& jointsVelocity,
                                                const FrameIndex fixedFrame,
                                                const Vector3& gravityInFixedFrame,
                                                const VectorDynSize& measurements);

        void updateEstimateInformationFloatingBase(const JointPosDoubleArray& jointsConfiguration,
                                                   const JointDOFsDoubleArray& jointsVelocity,
                                                   const FrameIndex floatingFrame,
                                                   const Vector3& bodyAngularVelocityOfSpecifiedFrame,
                                                   const VectorDynSize& measurements);

        void updateEstimateInformationFloatingBase(const Transform& baseTransform,
                                                   const JointPosDoubleArray& jointsConfiguration,
                                                   const JointDOFsDoubleArray& jointsVelocity,
                                                   const FrameIndex floatingFrame,
                                                   const Vector3& bodyAngularVelocityOfSpecifiedFrame,
                                                   const VectorDynSize& measurements,
                                                   bool& task1);

        bool doEstimate();
        bool doEstimate(const bool& task1);

        void getLastEstimate(iDynTree::VectorDynSize& lastEstimate) const;
        void getLastEstimate(iDynTree::VectorDynSize& lastEstimate, const bool task1) const;
        const iDynTree::VectorDynSize& getLastEstimate() const;


        const iDynTree::VectorDynSize& getSimulatedMeasurementVector(iDynTree::VectorDynSize& simulatedy, bool task1);

    };
}

#endif /* end of include guard: IDYNTREE_BERDY_SPARSEMAPSOLVER_H */