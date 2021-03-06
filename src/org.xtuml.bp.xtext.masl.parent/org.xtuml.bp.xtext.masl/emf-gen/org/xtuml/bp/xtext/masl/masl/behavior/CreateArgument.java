/**
 * generated by Xtext 2.9.2
 */
package org.xtuml.bp.xtext.masl.masl.behavior;

import org.eclipse.emf.ecore.EObject;

import org.xtuml.bp.xtext.masl.masl.structure.AttributeDefinition;
import org.xtuml.bp.xtext.masl.masl.structure.StateDeclaration;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Create Argument</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * </p>
 * <ul>
 *   <li>{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getAttribute <em>Attribute</em>}</li>
 *   <li>{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getValue <em>Value</em>}</li>
 *   <li>{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getCurrentState <em>Current State</em>}</li>
 * </ul>
 *
 * @see org.xtuml.bp.xtext.masl.masl.behavior.BehaviorPackage#getCreateArgument()
 * @model
 * @generated
 */
public interface CreateArgument extends EObject {
	/**
	 * Returns the value of the '<em><b>Attribute</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Attribute</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Attribute</em>' reference.
	 * @see #setAttribute(AttributeDefinition)
	 * @see org.xtuml.bp.xtext.masl.masl.behavior.BehaviorPackage#getCreateArgument_Attribute()
	 * @model
	 * @generated
	 */
	AttributeDefinition getAttribute();

	/**
	 * Sets the value of the '{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getAttribute <em>Attribute</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Attribute</em>' reference.
	 * @see #getAttribute()
	 * @generated
	 */
	void setAttribute(AttributeDefinition value);

	/**
	 * Returns the value of the '<em><b>Value</b></em>' containment reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Value</em>' containment reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Value</em>' containment reference.
	 * @see #setValue(Expression)
	 * @see org.xtuml.bp.xtext.masl.masl.behavior.BehaviorPackage#getCreateArgument_Value()
	 * @model containment="true"
	 * @generated
	 */
	Expression getValue();

	/**
	 * Sets the value of the '{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getValue <em>Value</em>}' containment reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Value</em>' containment reference.
	 * @see #getValue()
	 * @generated
	 */
	void setValue(Expression value);

	/**
	 * Returns the value of the '<em><b>Current State</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Current State</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Current State</em>' reference.
	 * @see #setCurrentState(StateDeclaration)
	 * @see org.xtuml.bp.xtext.masl.masl.behavior.BehaviorPackage#getCreateArgument_CurrentState()
	 * @model
	 * @generated
	 */
	StateDeclaration getCurrentState();

	/**
	 * Sets the value of the '{@link org.xtuml.bp.xtext.masl.masl.behavior.CreateArgument#getCurrentState <em>Current State</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Current State</em>' reference.
	 * @see #getCurrentState()
	 * @generated
	 */
	void setCurrentState(StateDeclaration value);

} // CreateArgument
